"""实时时钟 (RTC) 测试模块"""

from tests.base import TestCase
from gui.i18n import t


class RtcReadTest(TestCase):
    category_key = "cat_rtc"
    name_key = "tn_rtc_time_read"

    def _run(self):
        sudo_pw = getattr(self.board, "_password", "")
        rc, out, _ = self.cmd(f"echo '{sudo_pw}' | sudo -S /usr/sbin/hwclock -r 2>&1", timeout=10)
        if rc != 0:
            self.fail(t("rtc_read_fail", out.strip()))
        else:
            self.pass_(t("rtc_read_ok", out.strip()))


class RtcWriteReadTest(TestCase):
    category_key = "cat_rtc"
    name_key = "tn_rtc_write_read"

    def _run(self):
        sudo_pw = getattr(self.board, "_password", "")

        def sudo(cmd):
            return self.cmd(f"echo '{sudo_pw}' | sudo -S {cmd} 2>&1", timeout=15)

        rc, out, _ = sudo("/usr/sbin/hwclock --set --date='2020-06-15 10:30:00'")
        if rc != 0:
            self.fail(t("rtc_set_fail", out.strip()))
            return

        rc2, out2, _ = sudo("/usr/sbin/hwclock -r")
        if rc2 != 0:
            self.fail(t("rtc_readback_fail", out2.strip()))
            sudo("/usr/sbin/hwclock --systohc")
            return

        rtc_time = out2.strip()
        if "2020" not in rtc_time:
            self.fail(t("rtc_year_mismatch", rtc_time))
        else:
            self.pass_(t("rtc_write_ok", rtc_time))

        sudo("/usr/sbin/hwclock --systohc")


def get_tests(board, manual_confirm_fn=None, manual_input_fn=None, **kwargs):
    args = (board, manual_confirm_fn, manual_input_fn)
    return [
        RtcReadTest(*args),
        RtcWriteReadTest(*args),
    ]
