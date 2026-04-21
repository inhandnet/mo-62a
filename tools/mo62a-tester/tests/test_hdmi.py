"""HDMI 显示测试模块"""

import time

from tests.base import TestCase
from gui.i18n import t


_LIGHTDM_XAUTH = "/var/run/lightdm/root/:0"
_X_ENV = "DISPLAY=:0 XAUTHORITY=" + _LIGHTDM_XAUTH


def _sudo(tc: TestCase, cmd: str, timeout: int = 10):
    sudo_pw = getattr(tc.board, "_password", "")
    return tc.cmd(f"echo '{sudo_pw}' | sudo -S {cmd}", timeout=timeout)


class DrmDeviceTest(TestCase):
    category_key = "cat_hdmi"
    name_key = "tn_drm_node"

    def _run(self):
        rc, _, _ = self.cmd("ls /dev/dri/card0")
        if rc != 0:
            self.fail(t("hdmi_drm_missing"))
            return
        self.pass_(t("hdmi_drm_ok"))


class HdmiBridgeTest(TestCase):
    category_key = "cat_hdmi"
    name_key = "tn_sii902x_driver"

    def _run(self):
        rc, out, _ = self.cmd("journalctl -k --no-pager 2>/dev/null | grep -i 'sii902x' | tail -3")
        if rc != 0 or not out.strip():
            self.fail(t("hdmi_driver_missing"))
            return
        self.pass_(t("hdmi_driver_ok"))


class HdmiConnectedTest(TestCase):
    category_key = "cat_hdmi"
    name_key = "tn_hdmi_status"

    def _run(self):
        rc, out, _ = self.cmd(
            "cat /sys/class/drm/card*-HDMI*/status 2>/dev/null | head -1"
        )
        status = out.strip()
        if status != "connected":
            self.fail(t("hdmi_disconnected", status or "(none)"))
            return
        self.pass_(t("hdmi_connected"))


class EdidTest(TestCase):
    category_key = "cat_hdmi"
    name_key = "tn_edid_integrity"

    def _run(self):
        rc, out, _ = self.cmd("journalctl -k --no-pager 2>/dev/null | grep -i 'edid'")
        if "corrupt" in out.lower():
            self.fail(t("hdmi_edid_corrupt"))
            return
        self.pass_(t("hdmi_edid_ok"))


class ResolutionTest(TestCase):
    category_key = "cat_hdmi"
    name_key = "tn_display_resolution"

    def _run(self):
        rc, out, _ = self.cmd("DISPLAY=:0 xrandr 2>/dev/null | grep ' connected'")
        if rc != 0 or not out.strip():
            self.fail(t("hdmi_xrandr_fail"))
            return
        line = out.strip().splitlines()[0]
        self.pass_(line)


class ScreenshotCompareTest(TestCase):
    """截取设备桌面并在 Tester 中展示，让用户与 HDMI 显示器实际画面比对。"""
    category_key = "cat_hdmi"
    name_key = "tn_hdmi_screenshot"
    requires_manual = True

    def _run(self):
        # 检查 HDMI 是否连接
        _, status_out, _ = self.cmd(
            "cat /sys/class/drm/card*-HDMI*/status 2>/dev/null | head -1"
        )
        if status_out.strip() != "connected":
            self.fail(t("hdmi_ss_not_connected"))
            return

        # 用 sudo + lightdm root 认证唤醒显示器
        _sudo(self, f"env {_X_ENV} xset dpms force on 2>/dev/null", timeout=5)
        _sudo(self, f"env {_X_ENV} xset s reset 2>/dev/null", timeout=5)
        time.sleep(2)

        # 获取当前协商分辨率
        _, res_out, _ = _sudo(
            self,
            f"env {_X_ENV} xrandr 2>/dev/null | grep ' connected' | grep -o '[0-9]*x[0-9]*' | head -1",
        )
        resolution = res_out.strip() or "1280x800"

        # 截图
        rc, _, _ = _sudo(
            self,
            f"env {_X_ENV} ffmpeg -f x11grab -video_size {resolution} -i :0"
            f" -frames:v 1 -y /tmp/tester_ss.png 2>/dev/null",
            timeout=30,
        )
        if rc != 0:
            self.fail(t("hdmi_ss_fail", resolution))
            return

        # 通过 SFTP 下载 PNG
        try:
            image_bytes = self.board.get_file("/tmp/tester_ss.png")
        except Exception as e:
            self.fail(t("hdmi_download_fail", e))
            return
        finally:
            self.cmd("rm -f /tmp/tester_ss.png 2>/dev/null")

        if not image_bytes:
            self.fail(t("hdmi_ss_empty"))
            return

        ok = self.manual_image_confirm(
            t("hdmi_compare_prompt"),
            image_bytes,
        )
        if ok:
            self.pass_(t("hdmi_ss_match"))
        else:
            self.fail(t("hdmi_ss_mismatch"))


def get_tests(board, manual_confirm_fn=None, manual_input_fn=None, **kwargs):
    args = (board, manual_confirm_fn, manual_input_fn)
    return [
        ScreenshotCompareTest(*args),
    ]
