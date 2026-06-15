"""RTC 测试 — PCF85363

测试项：
  1. RTC 设备    — /dev/rtc0 存在，驱动名为 pcf85363
  2. 当前时间    — hwclock -r 读取，年份 >= 2024
  3. 时钟走动    — 间隔 2s 读两次，验证秒数有变化
  4. 写入验证    — 写入测试时间，读回对比，再从系统时钟恢复
"""

from __future__ import annotations
import re
import time
from interface.base import TestCase


# ── RTC 设备检测 ──────────────────────────────────────────────────────────────
class RtcDeviceTest(TestCase):
    category_key = "cat_rtc"
    name_key     = "tn_rtc_device"

    def _run(self):
        # 检查 /dev/rtc0
        rc, _, _ = self.cmd("test -c /dev/rtc0")
        if rc != 0:
            self.fail("/dev/rtc0 不存在")
            return

        # 确认是 PCF85363
        rc, out, _ = self.cmd("cat /sys/class/rtc/rtc0/name 2>/dev/null")
        name = out.strip()
        if "pcf85363" in name.lower():
            self.info(name)
        else:
            self.fail(f"驱动名称异常: {name}（期望 pcf85363）")


# ── 当前时间读取 ──────────────────────────────────────────────────────────────
class RtcReadTest(TestCase):
    category_key = "cat_rtc"
    name_key     = "tn_rtc_read"

    MIN_YEAR = 2024

    def _run(self):
        rc, out, err = self.cmd("hwclock -r")
        if rc != 0 or not out.strip():
            self.fail(f"hwclock -r 失败: {err.strip()[:60]}")
            return

        time_str = out.strip()

        # 从输出中提取年份，验证不是掉电归零
        m = re.search(r'(\d{4})', time_str)
        if not m or int(m.group(1)) < self.MIN_YEAR:
            self.fail(f"时间异常（可能掉电）: {time_str}")
            return

        self.info(time_str.split(".")[0])   # 去掉毫秒部分


# ── 时钟走动验证 ──────────────────────────────────────────────────────────────
class RtcTickTest(TestCase):
    category_key = "cat_rtc"
    name_key     = "tn_rtc_tick"

    WAIT_S  = 5
    TOL_MS  = 1500   # 误差容忍：1.5s（含网络往返延迟）

    def _run(self):
        # 读第一次 RTC 秒数，记录主机时间
        rc1, t1, _ = self.cmd("cat /sys/class/rtc/rtc0/time 2>/dev/null")
        host_t0 = time.monotonic()

        time.sleep(self.WAIT_S)

        # 读第二次 RTC 秒数，记录主机时间
        rc2, t2, _ = self.cmd("cat /sys/class/rtc/rtc0/time 2>/dev/null")
        host_elapsed_ms = (time.monotonic() - host_t0) * 1000

        if rc1 != 0 or rc2 != 0:
            self.fail("无法读取 RTC sysfs 时间")
            return

        def _secs(s: str) -> int | None:
            m = re.match(r'(\d{2}):(\d{2}):(\d{2})', s.strip())
            if m:
                return int(m.group(1)) * 3600 + int(m.group(2)) * 60 + int(m.group(3))
            return None

        s1, s2 = _secs(t1), _secs(t2)
        if s1 is None or s2 is None:
            self.fail(f"时间格式解析失败: {t1.strip()} / {t2.strip()}")
            return

        rtc_elapsed_ms = ((s2 - s1) % 86400) * 1000
        error_ms = abs(rtc_elapsed_ms - host_elapsed_ms)

        if error_ms <= self.TOL_MS:
            self.pass_(f"成功（误差 {error_ms:.0f}ms）")
        else:
            self.fail(f"失败（误差 {error_ms:.0f}ms）")


# ── 写入验证 ──────────────────────────────────────────────────────────────────
class RtcWriteTest(TestCase):
    category_key = "cat_rtc"
    name_key     = "tn_rtc_write"

    TEST_DATE = "2024-06-15 10:00:00"
    TEST_YEAR = "2024"

    def _run(self):
        # 写入测试时间
        rc, _, err = self.cmd(
            f"hwclock --set --date='{self.TEST_DATE}' 2>&1"
        )
        if rc != 0:
            self.fail(f"hwclock --set 失败: {err.strip()[:60]}")
            return

        # 读回验证
        rc, out, _ = self.cmd("hwclock -r")
        if rc != 0 or not out.strip():
            self.fail("写入后读取失败")
            return

        if self.TEST_YEAR not in out:
            self.fail(f"读回时间不符: {out.strip()}")
            return

        # 从系统时钟恢复 RTC
        self.cmd("hwclock -w 2>/dev/null")
        self.pass_(f"写入 {self.TEST_DATE}，读回正确，已从系统时钟恢复")


# ── 电池保持（硬件掉电）测试 ──────────────────────────────────────────────────
class RtcBatteryHoldTest(TestCase):
    """通过 TPS6594 PMIC RTC 闹钟唤醒，验证 PCF85263 电池保持时间。

    流程：
      1. 同步 RTC 到当前系统时间
      2. 读 PCF85263 时间 T1，记录主机时间 host_t1
      3. 设置 TPS6594 (rtc1) 唤醒闹钟为 T+WAKE_AFTER_S
      4. systemctl poweroff（PMIC 关电，VCC_3V3_SYS 切断，PCF85263 切电池）
      5. SSH 断开，主机等待 WAKE_AFTER_S + BOOT_BUDGET_S
      6. 轮询重连 SSH
      7. 读 PCF85263 时间 T2，记录主机时间 host_t2
      8. 验证 (T2 - T1) ≈ (host_t2 - host_t1)，误差 < 容差
    """
    category_key = "cat_rtc"
    name_key     = "tn_rtc_battery"

    WAKE_AFTER_S   = 30    # PMIC 唤醒延时
    BOOT_BUDGET_S  = 60    # 启动 + SSH 就绪预算
    TOL_S          = 5     # RTC 与实际经过时间的容差

    def _run(self):
        import time as _t

        # 查找支持 wakealarm 的 RTC，并且必须是 PMIC（TPS6594）域的 RTC
        # 否则关机后无法唤醒（K3 SoC 内部 RTC 会随 SoC 断电）
        rc, wake_rtc, _ = self.cmd(
            "for r in /sys/class/rtc/rtc*; do "
            "[ -e $r/wakealarm ] || continue; "
            "name=$(cat $r/name 2>/dev/null); "
            "case \"$name\" in *tps6594*|*tps659*) "
            "  echo $(basename $r); break;; "
            "esac; "
            "done"
        )
        wake_rtc = wake_rtc.strip()
        if not wake_rtc:
            self.skip(
                "未找到 TPS6594 PMIC RTC 作为唤醒源 "
                "（K3 SoC 内部 RTC 在关机后会一并断电，无法唤醒）"
            )
            return

        # 关闭 NTP，避免重启后从网络拉时间污染 RTC 读数
        _, ntp_was, _ = self.cmd("timedatectl show -p NTP --value 2>/dev/null")
        ntp_was = ntp_was.strip()
        self.cmd("timedatectl set-ntp false 2>/dev/null")

        try:
            # 同步 RTC 到当前系统时间
            self.cmd("hwclock -w 2>/dev/null")

            rc, out1, _ = self.cmd("hwclock -r")
            t1 = _parse_hwclock(out1)
            if t1 is None:
                self.fail("无法读取初始 RTC 时间")
                return
            host_t1 = _t.time()

            host = self.board.host
            user = self.board.user
            pwd  = self.board._password

            # 分两步：
            #   1. rtcwake -m no 只设闹钟，不睡眠
            #   2. systemctl poweroff 真正关机，PMIC 切 VCC_3V3_SYS
            rc, _, err = self.cmd(
                f"rtcwake -d /dev/{wake_rtc} -m no -s {self.WAKE_AFTER_S}"
            )
            if rc != 0:
                self.fail(f"rtcwake 设置闹钟失败: {err.strip()[:60]}")
                return

            # 后台 poweroff，让命令返回再断 SSH
            self.cmd("systemctl poweroff &", timeout=3)
            _t.sleep(2.0)
            try:
                self.board.close()
            except Exception:
                pass

            # 等待关机 + 唤醒 + 启动
            _t.sleep(self.WAKE_AFTER_S + 5)

            # 轮询重连 SSH
            deadline = _t.monotonic() + self.BOOT_BUDGET_S
            reconnected, last_err = False, ""
            while _t.monotonic() < deadline:
                try:
                    self.board.connect(host, user, pwd)
                    reconnected = True
                    break
                except Exception as e:
                    last_err = str(e)
                    _t.sleep(3.0)

            if not reconnected:
                self.fail(f"重连失败: {last_err[:60]}")
                return

            # 确保 NTP 仍关闭
            self.cmd("timedatectl set-ntp false 2>/dev/null")

            rc, out2, _ = self.cmd("hwclock -r")
            t2 = _parse_hwclock(out2)
            if t2 is None:
                self.fail("重启后无法读取 RTC 时间")
                return
            host_t2 = _t.time()

            rtc_elapsed  = t2 - t1
            host_elapsed = host_t2 - host_t1
            error_s      = abs(rtc_elapsed - host_elapsed)

            if error_s <= self.TOL_S:
                self.pass_(
                    f"成功（断电 {host_elapsed:.0f}s，RTC 走时 {rtc_elapsed:.0f}s，"
                    f"误差 {error_s:.1f}s）"
                )
            else:
                self.fail(
                    f"失败（断电 {host_elapsed:.0f}s，RTC 走时 {rtc_elapsed:.0f}s，"
                    f"误差 {error_s:.1f}s — 电池可能失效）"
                )
        finally:
            if ntp_was == "yes":
                try:
                    self.cmd("timedatectl set-ntp true 2>/dev/null")
                except Exception:
                    pass


def _parse_hwclock(out: str) -> float | None:
    """解析 hwclock -r 输出为 Unix 秒（含微秒）。"""
    from datetime import datetime
    # 典型格式: "2026-05-25 13:45:30.123456+08:00"
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
    return [
        RtcDeviceTest(board),
        RtcReadTest(board),
        RtcTickTest(board),
        RtcWriteTest(board),
        RtcBatteryHoldTest(board),
    ]
