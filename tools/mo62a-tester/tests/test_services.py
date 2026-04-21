"""系统服务测试模块"""

from tests.base import TestCase
from gui.i18n import t


class NoFailedServicesTest(TestCase):
    category_key = "cat_services"
    name_key = "tn_no_failed_services"

    def _run(self):
        rc, out, _ = self.cmd(
            "systemctl list-units --state=failed --no-legend --no-pager 2>/dev/null"
        )
        lines = [l for l in out.strip().splitlines() if l.strip()]
        if lines:
            self.fail(t("svc_failed_count", len(lines)))
        else:
            self.pass_(t("svc_no_failed"))


class SshServiceTest(TestCase):
    category_key = "cat_services"
    name_key = "tn_ssh_service"

    def _run(self):
        rc, out, _ = self.cmd("systemctl is-active ssh 2>/dev/null || systemctl is-active sshd 2>/dev/null")
        if out.strip() != "active":
            self.fail(t("svc_ssh_inactive", out.strip()))
        else:
            self.pass_(t("svc_ssh_ok"))


class NtpSyncTest(TestCase):
    category_key = "cat_services"
    name_key = "tn_ntp_sync"

    def _run(self):
        rc, out, _ = self.cmd(
            "timedatectl show --property=NTPSynchronized --value 2>/dev/null"
        )
        if out.strip() == "yes":
            self.pass_(t("svc_ntp_ok"))
            return
        rc2, out2, _ = self.cmd("systemctl is-active systemd-timesyncd 2>/dev/null")
        if out2.strip() == "active":
            self.pass_(t("svc_timesyncd_ok"))
        else:
            self.fail(t("svc_ntp_fail", out.strip() or "unknown"))


class LightdmServiceTest(TestCase):
    category_key = "cat_services"
    name_key = "tn_lightdm_service"

    def _run(self):
        rc, out, _ = self.cmd("systemctl is-active lightdm")
        if out.strip() != "active":
            self.fail(t("svc_lightdm_inactive", out.strip()))
        else:
            self.pass_(t("svc_lightdm_ok"))


class MoDiscoverTest(TestCase):
    category_key = "cat_services"
    name_key = "tn_mo_discover"

    def _run(self):
        rc, out, _ = self.cmd("systemctl is-active mo-discover")
        if out.strip() != "active":
            self.fail(t("svc_discover_inactive", out.strip()))
        else:
            self.pass_(t("svc_discover_ok"))


def get_tests(board, manual_confirm_fn=None, manual_input_fn=None, **kwargs):
    args = (board, manual_confirm_fn, manual_input_fn)
    return [
        NoFailedServicesTest(*args),
        SshServiceTest(*args),
        NtpSyncTest(*args),
        LightdmServiceTest(*args),
        MoDiscoverTest(*args),
    ]
