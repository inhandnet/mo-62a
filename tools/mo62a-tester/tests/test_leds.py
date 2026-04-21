"""LED 指示灯测试模块"""

from tests.base import TestCase
from gui.i18n import t


class LedVisualTest(TestCase):
    """停服务 → 红灯亮绿灯灭 → 红灯灭绿灯亮 → 恢复服务"""
    category_key = "cat_leds"
    name_key = "tn_led_visual"
    requires_manual = True

    def _run(self):
        sudo_pw = getattr(self.board, "_password", "")

        def sudo(cmd):
            self.cmd(f"echo '{sudo_pw}' | sudo -S {cmd}", timeout=10)

        # 停止 led-status 服务，接管 LED 控制权
        sudo("systemctl stop led-status")
        # 清除 trigger，确保直接控制
        sudo("bash -c 'echo none > /sys/class/leds/red/trigger; echo none > /sys/class/leds/green/trigger'")

        # GPIO_ACTIVE_LOW：brightness=0 → GPIO 低电平 → 灯亮；brightness=1 → 灯灭

        # 阶段 1：红灯亮，绿灯灭
        sudo("bash -c 'echo 1 > /sys/class/leds/red/brightness; echo 0 > /sys/class/leds/green/brightness'")
        ok1 = self.manual_confirm(t("led_confirm_red_on"))

        # 阶段 2：绿灯亮，红灯灭
        sudo("bash -c 'echo 0 > /sys/class/leds/red/brightness; echo 1 > /sys/class/leds/green/brightness'")
        ok2 = self.manual_confirm(t("led_confirm_green"))

        # 恢复 led-status 服务
        sudo("systemctl start led-status")

        if ok1 and ok2:
            self.pass_(t("led_visual_ok"))
        elif not ok1:
            self.fail(t("led_visual_fail_red"))
        else:
            self.fail(t("led_visual_fail_grn"))


def get_tests(board, manual_confirm_fn=None, manual_input_fn=None, **kwargs):
    args = (board, manual_confirm_fn, manual_input_fn)
    return [
        LedVisualTest(*args),
    ]
