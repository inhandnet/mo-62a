"""双色 LED 测试 — 改为人工确认

流程：
  1. 停止 led-status 服务，取得 LED 控制权
  2. 点亮红色 LED，弹窗让用户确认
  3. 点亮绿色 LED，弹窗让用户确认
  4. 关闭 LED，恢复 led-status 服务
"""

from __future__ import annotations
import time
from config.i18n import t
from interface.base import TestCase


class RedLedTest(TestCase):
    category_key = "cat_display"
    name_key     = "tn_led_red"

    def _run(self):
        _ensure_led_control(self)
        try:
            self.cmd("echo 1 > /sys/class/leds/red/brightness")
            self.cmd("echo 0 > /sys/class/leds/green/brightness")
            time.sleep(0.5)

            ok = self.manual_confirm("manual_led_red")
            if ok:
                self.pass_(t("msg_led_red_yes"))
            else:
                self.fail(t("msg_led_red_no"))
        finally:
            _restore_led_control(self)


class GreenLedTest(TestCase):
    category_key = "cat_display"
    name_key     = "tn_led_green"

    def _run(self):
        _ensure_led_control(self)
        try:
            self.cmd("echo 0 > /sys/class/leds/red/brightness")
            self.cmd("echo 1 > /sys/class/leds/green/brightness")
            time.sleep(0.5)

            ok = self.manual_confirm("manual_led_green")
            if ok:
                self.pass_(t("msg_led_green_yes"))
            else:
                self.fail(t("msg_led_green_no"))
        finally:
            _restore_led_control(self)


# 共享工具函数（避免重复启停服务）
def _ensure_led_control(test: TestCase):
    test.cmd("systemctl stop led-status 2>/dev/null")
    test.cmd("echo none > /sys/class/leds/red/trigger 2>/dev/null")
    test.cmd("echo none > /sys/class/leds/green/trigger 2>/dev/null")


def _restore_led_control(test: TestCase):
    test.cmd("echo 0 > /sys/class/leds/red/brightness 2>/dev/null")
    test.cmd("echo 0 > /sys/class/leds/green/brightness 2>/dev/null")
    test.cmd("systemctl start led-status 2>/dev/null")


def get_tests(board) -> list:
    return [
        RedLedTest(board),
        GreenLedTest(board),
    ]
