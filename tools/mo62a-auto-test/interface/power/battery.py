"""电池保持测试 — 人工断电/上电版本

流程：
  1. 读取当前 RTC 时间 T1，记录主机时间 host_t1
  2. 弹出无按钮提示对话框，提示用户对设备断电
  3. 后台通过 ICMP ping 探测设备是否离线（证明已断电）
  4. ping 不通后立即关闭 SSH，等待 10 秒
  5. 弹出无按钮提示对话框，提示用户对设备上电
  6. 循环 ping，ping 通后重建 SSH
  7. 连上后读取 RTC 时间 T2，记录主机时间 host_t2
  8. 验证 (T2 - T1) ≈ 10s，误差 < 容差
"""

from __future__ import annotations
import platform
import re
import subprocess
import time as _t
from config.i18n import t
from interface.base import TestCase


class BatteryHoldTest(TestCase):
    category_key = "cat_power"
    name_key     = "tn_rtc_battery"

    POWER_OFF_WAIT_S = 10     # 断电后等待时间
    BOOT_BUDGET_S    = 60     # 上电后 ping 通预算
    TOL_S            = 5      # RTC 与实际经过时间的容差
    PING_TIMEOUT_S   = 1      # 单次 ping 超时

    def _run(self):
        # 同步 RTC 到系统时间，确保起始时间准确
        self.cmd("hwclock -w 2>/dev/null")

        rc, out1, _ = self.cmd("hwclock -r")
        t1 = _parse_hwclock(out1)
        if t1 is None:
            self.fail(t("msg_battery_rtc_read_fail"))
            return
        host_t1 = _t.time()

        host = self.board.host
        user = self.board.user
        pwd  = self.board._password

        # 1) 提示用户断电（无进度条）
        close_dlg = self.manual_prompt("manual_battery_disconnect", show_progress=False)
        try:
            # 2) 通过 ICMP ping 探测设备是否离线
            disconnect_deadline = _t.monotonic() + 20
            disconnected = False
            while _t.monotonic() < disconnect_deadline:
                if not _ping_host(host, self.PING_TIMEOUT_S):
                    disconnected = True
                    break
                _t.sleep(0.5)

            if not disconnected:
                self.fail(t("msg_battery_no_poweroff"))
                return

            # ping 不通说明已断电，立即关闭 SSH 连接
            try:
                self.board.close()
            except Exception:
                pass
        finally:
            if close_dlg:
                close_dlg()

        # 3) 断电后等待 10 秒
        _t.sleep(self.POWER_OFF_WAIT_S)

        # 4) 提示用户上电（无进度条）
        close_dlg2 = self.manual_prompt("manual_battery_reconnect", show_progress=False)
        try:
            # 5) 循环 ping，ping 通后重建 SSH
            reconnected = False
            deadline = _t.monotonic() + self.BOOT_BUDGET_S
            while _t.monotonic() < deadline:
                if _ping_host(host, self.PING_TIMEOUT_S):
                    try:
                        self.board.connect(host, user, pwd)
                        reconnected = True
                        break
                    except Exception:
                        # ping 通但 SSH 还没就绪，稍后再试
                        pass
                _t.sleep(2.0)

            if not reconnected:
                self.fail(t("msg_battery_reconnect_fail"))
                return
        finally:
            if close_dlg2:
                close_dlg2()

        # 6) 读取 RTC 时间并校验
        rc, out2, _ = self.cmd("hwclock -r")
        t2 = _parse_hwclock(out2)
        if t2 is None:
            self.fail(t("msg_battery_rtc_readback_fail"))
            return
        host_t2 = _t.time()

        rtc_elapsed  = t2 - t1
        host_elapsed = host_t2 - host_t1
        error_s      = abs(rtc_elapsed - host_elapsed)

        if error_s <= self.TOL_S:
            self.pass_(
                t("msg_battery_pass", f"{host_elapsed:.0f}", f"{rtc_elapsed:.0f}", f"{error_s:.1f}")
            )
        else:
            self.fail(
                t("msg_battery_fail", f"{host_elapsed:.0f}", f"{rtc_elapsed:.0f}", f"{error_s:.1f}")
            )


def _ping_host(host: str, timeout_s: int) -> bool:
    """返回设备是否在线（True=在线）。兼容 Windows / Linux。"""
    system = platform.system().lower()
    if system == "windows":
        cmd = ["ping", "-n", "1", "-w", str(int(timeout_s * 1000)), host]
    else:
        cmd = ["ping", "-c", "1", "-W", str(timeout_s), host]

    kwargs = {
        "stdout": subprocess.DEVNULL,
        "stderr": subprocess.DEVNULL,
        "timeout": timeout_s + 2,
    }
    if system == "windows":
        kwargs["creationflags"] = subprocess.CREATE_NO_WINDOW

    try:
        result = subprocess.run(cmd, **kwargs)
        return result.returncode == 0
    except Exception:
        return False


def _parse_hwclock(out: str) -> float | None:
    """解析 hwclock -r 输出为 Unix 秒（含微秒）。"""
    from datetime import datetime
    m = re.search(
        r'(\d{4})-(\d{2})-(\d{2})\s+(\d{2}):(\d{2}):(\d{2})(?:\.(\d+))?',
        out
    )
    if not m:
        return None
    y, mo, d, hh, mi, se = (int(m.group(i)) for i in range(1, 7))
    us = int((m.group(7) or "0")[:6].ljust(6, "0"))
    try:
        return datetime(y, mo, d, hh, mi, se, us).timestamp()
    except Exception:
        return None


def get_tests(board) -> list:
    return [BatteryHoldTest(board)]
