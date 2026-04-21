"""风扇控制测试模块"""

from tests.base import TestCase
from gui.i18n import t


class FanServiceTest(TestCase):
    category = "风扇控制"
    name_key = "tn_fancontrol_service"

    def _run(self):
        rc, out, _ = self.cmd("systemctl is-active fancontrol")
        if out.strip() != "active":
            self.fail(t("fan_svc_inactive", out.strip()))
        else:
            self.pass_(t("fan_svc_ok"))


class TempReadTest(TestCase):
    category = "风扇控制"
    name_key = "tn_hwmon_temp"

    def _run(self):
        rc, out, _ = self.cmd(
            "cat /sys/class/hwmon/hwmon*/temp1_input 2>/dev/null | head -1"
        )
        raw = out.strip()
        if not raw:
            self.fail(t("fan_temp_missing"))
            return
        try:
            temp_c = int(raw) / 1000.0
        except ValueError:
            self.fail(t("fan_temp_parse_fail", raw))
            return
        self.pass_(t("fan_temp_ok", temp_c))


class FanPwmSliderTest(TestCase):
    """停止 fancontrol → 滑动条手动控制 PWM（10~100%）→ 恢复 fancontrol"""
    category = "风扇控制"
    name_key = "tn_fan_visual"
    requires_manual = True

    def _run(self):
        sudo_pw = getattr(self.board, "_password", "")

        def sudo(cmd, timeout=10):
            rc, out, _ = self.cmd(f"echo '{sudo_pw}' | sudo -S {cmd} 2>&1", timeout=timeout)
            return rc, out

        # 查找 pwmfan hwmon 路径
        rc, out, _ = self.cmd(
            "for d in /sys/class/hwmon/hwmon*; do "
            "[ \"$(cat $d/name 2>/dev/null)\" = 'pwmfan' ] && echo $d && break; done"
        )
        hwmon_path = out.strip()
        if not hwmon_path:
            self.fail(t("fan_hwmon_missing"))
            return

        pwm_path = f"{hwmon_path}/pwm1"
        enable_path = f"{hwmon_path}/pwm1_enable"

        # 停止 fancontrol，切换为手动模式，初始 50%
        sudo("systemctl stop fancontrol")
        sudo(f"sh -c 'echo 1 > {enable_path}'")
        sudo(f"sh -c 'echo 128 > {pwm_path}'")

        def on_change(pct):
            pwm_val = max(0, min(255, round(pct / 100 * 255)))
            sudo(f"sh -c 'echo {pwm_val} > {pwm_path}'", timeout=5)

        ok = self.manual_slider_confirm(
            t("fan_slider_prompt"),
            min_val=10, max_val=100,
            on_change=on_change,
            initial_val=50,
        )

        # 恢复 fancontrol
        sudo("systemctl start fancontrol")

        if ok:
            self.pass_(t("fan_visual_ok"))
        else:
            self.fail(t("fan_visual_fail"))


def get_tests(board, manual_confirm_fn=None, manual_input_fn=None, **kwargs):
    args = (board, manual_confirm_fn, manual_input_fn)
    return [
        FanServiceTest(*args),
        TempReadTest(*args),
        FanPwmSliderTest(*args),
    ]
