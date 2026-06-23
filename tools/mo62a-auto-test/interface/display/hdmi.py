"""HDMI 显示测试 — 改为人工确认

测试项：
  1. HDMI 状态  — 已连接显示分辨率（INFO）
  2. HDMI 画面  — 重启 lightdm 后由用户确认是否显示登录界面（人工 PASS/FAIL）
"""

from __future__ import annotations
import time
from config.i18n import t
from interface.base import TestCase


# ── HDMI 状态（连接 + 分辨率）─────────────────────────────────────────────────
class HdmiStatusTest(TestCase):
    category_key = "cat_display"
    name_key     = "tn_hdmi_status"

    def _run(self):
        rc, status_out, _ = self.cmd(
            "cat /sys/class/drm/card*-HDMI*/status 2>/dev/null | head -1"
        )
        if rc != 0 or not status_out.strip():
            self.fail(t("msg_hdmi_drm_missing"))
            return

        if status_out.strip() != "connected":
            self.info(t("msg_hdmi_not_connected"))
            return

        rc, modes_out, _ = self.cmd(
            "cat /sys/class/drm/card*-HDMI*/modes 2>/dev/null | head -1"
        )
        resolution = modes_out.strip() if (rc == 0 and modes_out.strip()) else "unknown"
        self.info(resolution)


# ── HDMI 画面验证（人工确认登录界面）──────────────────────────────────────────
class HdmiScreenTest(TestCase):
    category_key = "cat_display"
    name_key     = "tn_hdmi_screen"

    def _run(self):
        # 先确认 HDMI 已连接
        rc, status_out, _ = self.cmd(
            "cat /sys/class/drm/card*-HDMI*/status 2>/dev/null | head -1"
        )
        if rc != 0 or status_out.strip() != "connected":
            self.skip(t("msg_hdmi_not_connected"))
            return

        # 重启 lightdm，让登录界面显示在 HDMI 上
        self.cmd("systemctl restart lightdm", timeout=20)
        time.sleep(2.0)

        ok = self.manual_confirm("manual_hdmi_login")
        if ok:
            self.pass_(t("msg_hdmi_user_yes"))
        else:
            self.fail(t("msg_hdmi_user_no"))


def get_tests(board) -> list:
    return [
        HdmiStatusTest(board),
        HdmiScreenTest(board),
    ]
