"""
HDMI 显示测试模块
"""

from tests.base import TestCase, TestResult


class DrmDeviceTest(TestCase):
    category = "HDMI 显示"
    name_key = "tn_drm_node"

    def _run(self):
        rc, out, err = self.cmd("ls /dev/dri/card0")
        if rc != 0:
            self.fail("/dev/dri/card0 不存在")
            return
        self.pass_("/dev/dri/card0 存在")


class HdmiBridgeTest(TestCase):
    category = "HDMI 显示"
    name_key = "tn_sii902x_driver"

    def _run(self):
        rc, out, err = self.cmd("dmesg | grep -i 'sii902x'")
        if "attached" not in out.lower():
            self.fail("dmesg 中未找到 sii902x attached 记录")
            return
        self.pass_("SiI902x HDMI 桥接已 attached")


class HdmiConnectedTest(TestCase):
    category = "HDMI 显示"
    name_key = "tn_hdmi_status"

    def _run(self):
        rc, out, err = self.cmd(
            "cat /sys/class/drm/card*-HDMI*/status 2>/dev/null | head -1"
        )
        status = out.strip()
        if status != "connected":
            self.fail(
                f"HDMI 连接状态：{status or '(无输出)'}，请确认 HDMI 线已连接"
            )
            return
        self.pass_("HDMI 已连接")


class EdidTest(TestCase):
    category = "HDMI 显示"
    name_key = "tn_edid_integrity"

    def _run(self):
        rc, out, err = self.cmd("dmesg | grep -i 'edid'")
        if "corrupt" in out.lower():
            self.fail("dmesg 中检测到 EDID corrupt 错误")
            return
        self.pass_("EDID 无 corrupt 错误")


class ResolutionTest(TestCase):
    category = "HDMI 显示"
    name_key = "tn_display_resolution"

    def _run(self):
        rc, out, err = self.cmd("xrandr --display :0 2>/dev/null | grep ' connected'")
        if rc != 0 or not out.strip():
            self.fail("xrandr 未返回已连接显示器信息")
            return
        # 提取分辨率信息（第一个连接的输出）
        resolution_line = out.strip().splitlines()[0]
        self.pass_(f"分辨率信息：{resolution_line}")


class DpmsVisualTest(TestCase):
    category = "HDMI 显示"
    name_key = "tn_dpms_visual"
    requires_manual = True

    def _run(self):
        # 息屏
        self.cmd("DISPLAY=:0 xset dpms force off")
        confirmed = self.manual_confirm(
            "请确认：HDMI 屏幕已息屏（变黑）"
        )
        # 无论结果如何，先唤醒屏幕
        self.cmd("DISPLAY=:0 xset dpms force on")
        if not confirmed:
            return
        self.pass_("HDMI DPMS 息屏目视确认通过，屏幕已唤醒")


def get_tests(board, manual_confirm_fn=None):
    return [
        DrmDeviceTest(board, manual_confirm_fn),
        HdmiBridgeTest(board, manual_confirm_fn),
        HdmiConnectedTest(board, manual_confirm_fn),
        EdidTest(board, manual_confirm_fn),
        ResolutionTest(board, manual_confirm_fn),
        DpmsVisualTest(board, manual_confirm_fn),
    ]
