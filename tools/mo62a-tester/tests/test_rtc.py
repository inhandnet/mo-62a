"""
实时时钟 (RTC) 测试模块
"""

from tests.base import TestCase, TestResult


class RtcDriverTest(TestCase):
    category = "实时时钟 (RTC)"
    name_key = "tn_rtc_node"

    def _run(self):
        rc, out, err = self.cmd("ls /dev/rtc0")
        if rc != 0:
            self.fail("/dev/rtc0 不存在")
            return
        self.pass_("/dev/rtc0 存在")


class RtcModuleTest(TestCase):
    category = "实时时钟 (RTC)"
    name_key = "tn_rtc_module"

    def _run(self):
        if not self.assert_contains(
            "lsmod | grep rtc_pcf85363",
            "rtc_pcf85363",
            "rtc_pcf85363 模块未加载",
        ):
            return
        self.pass_("rtc_pcf85363 模块已加载")


class RtcReadTest(TestCase):
    category = "实时时钟 (RTC)"
    name_key = "tn_rtc_time_read"

    def _run(self):
        rc, out, err = self.cmd("hwclock -r")
        if rc != 0:
            self.fail(f"hwclock -r 失败（rc={rc}）：{err.strip()}")
            return
        self.pass_(f"当前 RTC 时间：{out.strip()}")


class RtcWriteReadTest(TestCase):
    category = "实时时钟 (RTC)"
    name_key = "tn_rtc_write_read"

    def _run(self):
        # 写入固定时间
        rc, out, err = self.cmd('hwclock --set --date="2026-01-01 12:00:00"')
        if rc != 0:
            self.fail(f"hwclock --set 失败（rc={rc}）：{err.strip()}")
            return

        # 读回并验证年份
        rc2, out2, err2 = self.cmd("hwclock -r")
        if rc2 != 0:
            self.fail(f"写入后 hwclock -r 失败（rc={rc2}）：{err2.strip()}")
            # 尝试恢复
            self.cmd("hwclock --hctosys")
            return

        rtc_time = out2.strip()
        if "2026" not in rtc_time:
            self.fail(f"RTC 读回时间年份不符（期望 2026）：{rtc_time}")
            self.cmd("hwclock --hctosys")
            return

        # 恢复系统时间
        self.cmd("hwclock --hctosys")
        self.pass_(f"RTC 写入读回验证通过，读回时间：{rtc_time}")


def get_tests(board, manual_confirm_fn=None):
    return [
        RtcDriverTest(board, manual_confirm_fn),
        RtcModuleTest(board, manual_confirm_fn),
        RtcReadTest(board, manual_confirm_fn),
        RtcWriteReadTest(board, manual_confirm_fn),
    ]
