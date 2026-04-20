"""
风扇控制测试模块
"""

from tests.base import TestCase, TestResult


class FanServiceTest(TestCase):
    category = "风扇控制"
    name_key = "tn_fancontrol_service"

    def _run(self):
        rc, out, err = self.cmd("systemctl is-active fancontrol")
        status = out.strip()
        if status != "active":
            self.fail(f"fancontrol 服务未运行（状态：{status}）")
            return
        self.pass_("fancontrol 服务正在运行")


class PwmChipTest(TestCase):
    category = "风扇控制"
    name_key = "tn_pwm_chip"

    def _run(self):
        rc, out, err = self.cmd("ls /sys/class/pwm/pwmchip0/")
        if rc != 0:
            self.fail(f"pwmchip0 节点不存在：{err.strip()}")
            return
        self.pass_("pwmchip0 节点存在")


class TempReadTest(TestCase):
    category = "风扇控制"
    name_key = "tn_hwmon_temp"

    def _run(self):
        rc, out, err = self.cmd(
            "cat /sys/class/hwmon/hwmon*/temp1_input 2>/dev/null | head -1"
        )
        raw = out.strip()
        if not raw:
            self.fail("未能从 hwmon 读取温度（无输出）")
            return
        try:
            temp_raw = int(raw)
        except ValueError:
            self.fail(f"无法解析 hwmon 温度值：{raw}")
            return
        if temp_raw <= 0:
            self.fail(f"hwmon 温度值异常：{temp_raw}")
            return
        temp_c = temp_raw / 1000.0
        self.pass_(f"hwmon 温度：{temp_c:.1f}°C")


class FanVisualTest(TestCase):
    category = "风扇控制"
    name_key = "tn_fan_visual"
    requires_manual = True

    def _run(self):
        confirmed = self.manual_confirm(
            "请确认：风扇正在运转（可用手感受气流）"
        )
        if not confirmed:
            return
        self.pass_("风扇目视/触感确认运转正常")


def get_tests(board, manual_confirm_fn=None):
    return [
        FanServiceTest(board, manual_confirm_fn),
        PwmChipTest(board, manual_confirm_fn),
        TempReadTest(board, manual_confirm_fn),
        FanVisualTest(board, manual_confirm_fn),
    ]
