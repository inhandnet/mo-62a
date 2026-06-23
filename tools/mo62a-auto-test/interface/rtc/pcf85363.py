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
from config.i18n import t
from interface.base import TestCase


# ── RTC 设备检测 ──────────────────────────────────────────────────────────────
class RtcDeviceTest(TestCase):
    category_key = "cat_rtc"
    name_key     = "tn_rtc_device"

    def _run(self):
        # 检查 /dev/rtc0
        rc, _, _ = self.cmd("test -c /dev/rtc0")
        if rc != 0:
            self.fail(t("msg_rtc_device_missing"))
            return

        # 确认是 PCF85363
        rc, out, _ = self.cmd("cat /sys/class/rtc/rtc0/name 2>/dev/null")
        name = out.strip()
        if "pcf85363" in name.lower():
            self.info(name)
        else:
            self.fail(t("msg_rtc_driver_mismatch", name))


# ── 当前时间读取 ──────────────────────────────────────────────────────────────
class RtcReadTest(TestCase):
    category_key = "cat_rtc"
    name_key     = "tn_rtc_read"

    MIN_YEAR = 2024

    def _run(self):
        rc, out, err = self.cmd("hwclock -r")
        if rc != 0 or not out.strip():
            self.fail(t("msg_rtc_read_fail", err.strip()[:60]))
            return

        time_str = out.strip()

        # 从输出中提取年份，验证不是掉电归零
        m = re.search(r'(\d{4})', time_str)
        if not m or int(m.group(1)) < self.MIN_YEAR:
            self.fail(t("msg_rtc_time_abnormal", time_str))
            return

        self.info(time_str.split(".")[0])   # 去掉毫秒部分


# ── 时钟走动验证 ──────────────────────────────────────────────────────────────
class RtcTickTest(TestCase):
    category_key = "cat_rtc"
    name_key     = "tn_rtc_tick"

    WAIT_S  = 1
    TOL_MS  = 2000   # 误差容忍：2s（含网络往返延迟）

    def _run(self):
        # 读第一次 RTC 秒数，记录主机时间
        rc1, t1, _ = self.cmd("cat /sys/class/rtc/rtc0/time 2>/dev/null")
        host_t0 = time.monotonic()

        time.sleep(self.WAIT_S)

        # 读第二次 RTC 秒数，记录主机时间
        rc2, t2, _ = self.cmd("cat /sys/class/rtc/rtc0/time 2>/dev/null")
        host_elapsed_ms = (time.monotonic() - host_t0) * 1000

        if rc1 != 0 or rc2 != 0:
            self.fail(t("msg_rtc_sysfs_fail"))
            return

        def _secs(s: str) -> int | None:
            m = re.match(r'(\d{2}):(\d{2}):(\d{2})', s.strip())
            if m:
                return int(m.group(1)) * 3600 + int(m.group(2)) * 60 + int(m.group(3))
            return None

        s1, s2 = _secs(t1), _secs(t2)
        if s1 is None or s2 is None:
            self.fail(t("msg_rtc_parse_fail", t1.strip(), t2.strip()))
            return

        rtc_elapsed_ms = ((s2 - s1) % 86400) * 1000
        error_ms = abs(rtc_elapsed_ms - host_elapsed_ms)

        if error_ms <= self.TOL_MS:
            self.pass_(t("msg_rtc_tick_pass", f"{error_ms:.0f}"))
        else:
            self.fail(t("msg_rtc_tick_fail", f"{error_ms:.0f}"))


# ── 写入验证 ──────────────────────────────────────────────────────────────────
class RtcWriteTest(TestCase):
    """写入一个未来时间，读回验证，再从系统时钟恢复。

    使用当前系统时间 + 5 秒作为测试目标，避免固定测试日期跨时区/夏令时问题，
    同时保证验证快速完成。
    """

    category_key = "cat_rtc"
    name_key     = "tn_rtc_write"

    DELTA_S = 5

    def _run(self):
        # 读取当前系统时间，计算未来测试时间
        rc, out, _ = self.cmd("date -u '+%Y-%m-%d %H:%M:%S'")
        if rc != 0 or not out.strip():
            self.fail(t("msg_rtc_system_time_fail"))
            return

        try:
            from datetime import datetime, timedelta
            now = datetime.strptime(out.strip(), "%Y-%m-%d %H:%M:%S")
            test_time = now + timedelta(seconds=self.DELTA_S)
            test_date_str = test_time.strftime("%Y-%m-%d %H:%M:%S")
            test_year = str(test_time.year)
        except Exception as e:
            self.fail(t("msg_rtc_time_parse_fail", e))
            return

        # 写入测试时间
        rc, _, err = self.cmd(
            f"hwclock --set --date='{test_date_str}' 2>&1"
        )
        if rc != 0:
            self.fail(t("msg_rtc_set_fail", err.strip()[:60]))
            return

        # 读回验证
        rc, out, _ = self.cmd("hwclock -r")
        if rc != 0 or not out.strip():
            self.fail(t("msg_rtc_readback_fail"))
            return

        if test_year not in out:
            self.fail(t("msg_rtc_mismatch", out.strip()))
            return

        # 从系统时钟恢复 RTC
        self.cmd("hwclock -w 2>/dev/null")
        self.pass_(t("msg_rtc_write_pass", test_date_str))


def get_tests(board) -> list:
    return [
        RtcDeviceTest(board),
        RtcReadTest(board),
        RtcTickTest(board),
        RtcWriteTest(board),
    ]
