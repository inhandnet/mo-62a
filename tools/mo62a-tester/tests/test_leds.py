"""
LED 指示灯测试模块
"""

from tests.base import TestCase, TestResult


def _get_led_name(board, color_keywords: list[str]) -> str:
    """从 /sys/class/leds/ 动态获取匹配颜色关键字的 LED 名称。"""
    rc, out, err = board.run("ls /sys/class/leds/")
    if rc != 0:
        return ""
    for line in out.strip().splitlines():
        for kw in color_keywords:
            if kw.lower() in line.lower():
                return line.strip()
    return ""


class RedLedNodeTest(TestCase):
    category = "LED 指示灯"
    name_key = "tn_red_led_node"

    def _run(self):
        if not self.assert_contains(
            "ls /sys/class/leds/ | grep -i red",
            "red",
            "未找到红色 LED sysfs 节点",
        ):
            return
        self.pass_("红色 LED 节点存在")


class GreenLedNodeTest(TestCase):
    category = "LED 指示灯"
    name_key = "tn_green_led_node"

    def _run(self):
        rc, out, err = self.cmd("ls /sys/class/leds/")
        if rc != 0:
            self.fail(f"无法列出 LED 节点：{err.strip()}")
            return
        # 接受 green 或 act（ACT LED 通常为绿色）
        found = [
            line.strip()
            for line in out.strip().splitlines()
            if "green" in line.lower() or "act" in line.lower()
        ]
        if not found:
            self.fail("未找到绿色 LED（green/act）sysfs 节点")
            return
        self.pass_(f"绿色 LED 节点：{', '.join(found)}")


class LedServiceTest(TestCase):
    category = "LED 指示灯"
    name_key = "tn_led_service"

    def _run(self):
        rc, out, err = self.cmd("systemctl is-active led-status")
        status = out.strip()
        if status != "active":
            self.fail(f"led-status 服务未运行（状态：{status}）")
            return
        self.pass_("led-status 服务正在运行")


class RedLedVisualTest(TestCase):
    category = "LED 指示灯"
    name_key = "tn_red_led_visual"
    requires_manual = True

    def _run(self):
        # 动态获取红色 LED 名称
        led_name = _get_led_name(self.board, ["red"])
        if not led_name:
            self.skip("未找到红色 LED 节点，跳过目视测试")
            return

        self.cmd(f"echo 1 > /sys/class/leds/{led_name}/brightness")
        confirmed = self.manual_confirm(
            f"请确认：红色 LED（PWR，节点 {led_name}）已点亮"
        )
        # 恢复
        self.cmd(f"echo 0 > /sys/class/leds/{led_name}/brightness")
        if not confirmed:
            return  # status 已由 manual_confirm 设置
        self.pass_(f"红色 LED（{led_name}）目视确认点亮")


class GreenLedVisualTest(TestCase):
    category = "LED 指示灯"
    name_key = "tn_green_led_visual"
    requires_manual = True

    def _run(self):
        confirmed = self.manual_confirm(
            "请确认：绿色 LED（ACT）正在呼吸闪烁"
        )
        if not confirmed:
            return
        self.pass_("绿色 LED（ACT）目视确认正在闪烁")


def get_tests(board, manual_confirm_fn=None):
    return [
        RedLedNodeTest(board, manual_confirm_fn),
        GreenLedNodeTest(board, manual_confirm_fn),
        LedServiceTest(board, manual_confirm_fn),
        RedLedVisualTest(board, manual_confirm_fn),
        GreenLedVisualTest(board, manual_confirm_fn),
    ]
