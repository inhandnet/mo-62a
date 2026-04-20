"""
摄像头 (IMX219) 测试模块
"""

from tests.base import TestCase, TestResult


class VideoNodeTest(TestCase):
    category = "摄像头 (IMX219)"
    name_key = "tn_video_node"

    def _run(self):
        rc, out, err = self.cmd("ls /dev/video* 2>/dev/null")
        if rc != 0 or not out.strip():
            self.fail("未找到 /dev/video* 设备节点")
            return
        nodes = out.strip()
        self.pass_(f"视频节点：{nodes}")


class I2c2Imx219Test(TestCase):
    category = "摄像头 (IMX219)"
    name_key = "tn_imx219_sensor"

    def _run(self):
        rc, out, err = self.cmd("i2cdetect -y 2 2>/dev/null")
        if "10" not in out:
            self.fail("I2C-2 上未检测到 IMX219（地址 0x10）")
            return
        self.pass_("I2C-2 检测到 IMX219（0x10）")


class V4l2FormatsTest(TestCase):
    category = "摄像头 (IMX219)"
    name_key = "tn_v4l2_format"

    def _run(self):
        if not self.assert_contains(
            "v4l2-ctl --list-formats-ext 2>/dev/null",
            "SRGGB10",
            "未找到 SRGGB10 格式（IMX219 原始格式）",
        ):
            return
        self.pass_("检测到 SRGGB10 格式支持")


class CaptureTest(TestCase):
    category = "摄像头 (IMX219)"
    name_key = "tn_video_capture"

    def _run(self):
        rc, out, err = self.cmd(
            "v4l2-ctl --device=/dev/video0 --stream-mmap --stream-count=1 2>/dev/null",
            timeout=15,
        )
        if rc != 0:
            self.fail(f"v4l2-ctl 捕获帧失败（rc={rc}）：{err.strip()}")
            return
        self.pass_("成功捕获 1 帧视频")


def get_tests(board, manual_confirm_fn=None):
    return [
        VideoNodeTest(board, manual_confirm_fn),
        I2c2Imx219Test(board, manual_confirm_fn),
        V4l2FormatsTest(board, manual_confirm_fn),
        CaptureTest(board, manual_confirm_fn),
    ]
