"""HDMI 显示测试

测试项：
  1. HDMI 状态  — 已连接显示分辨率（如 1920x1080），未连接显示"未连接"
  2. HDMI 画面  — USB 摄像头验证 R / G / B 三色画面（PASS/FAIL）
                  自动从多路摄像头中识别对准显示器的那一路
                  每个颜色保存一张拍摄结果到 reports/
"""

from __future__ import annotations
import time
from datetime import datetime
from pathlib import Path
from interface.base import TestCase

# 颜色测试：(名称, BGRA字节, 判断函数(r,g,b)->bool)
# framebuffer 32bpp BGRA 字节顺序：[Blue, Green, Red, Alpha]
def _dominant(ch, r, g, b):
    total = r + g + b + 1e-3
    return (ch / total) > 0.45

_COLOR_TESTS = [
    ("R", [0,   0,   255, 255], lambda r, g, b: _dominant(r, r, g, b)),
    ("G", [0,   255, 0,   255], lambda r, g, b: _dominant(g, r, g, b)),
    ("B", [255, 0,   0,   255], lambda r, g, b: _dominant(b, r, g, b)),
]

_FB_WRITE_SCRIPT = (
    "python3 -c \""
    "with open('/sys/class/graphics/fb0/virtual_size') as f: w,h=map(int,f.read().strip().split(','));"
    "color=bytes({bgra});"
    "open('/dev/fb0','wb').write(color*(w*h))"
    "\""
)


# ── HDMI 状态（连接 + 分辨率）─────────────────────────────────────────────────
class HdmiStatusTest(TestCase):
    category_key = "cat_display"
    name_key     = "tn_hdmi_status"

    def _run(self):
        rc, status_out, _ = self.cmd(
            "cat /sys/class/drm/card*-HDMI*/status 2>/dev/null | head -1"
        )
        if rc != 0 or not status_out.strip():
            self.fail("未找到 HDMI DRM 节点")
            return

        if status_out.strip() != "connected":
            self.info("未连接")
            return

        rc, modes_out, _ = self.cmd(
            "cat /sys/class/drm/card*-HDMI*/modes 2>/dev/null | head -1"
        )
        resolution = modes_out.strip() if (rc == 0 and modes_out.strip()) else "unknown"
        self.info(resolution)


# ── HDMI 画面验证（USB 摄像头）────────────────────────────────────────────────
class HdmiScreenTest(TestCase):
    category_key = "cat_display"
    name_key     = "tn_hdmi_screen"

    COLOR_WAIT_S = 1.5

    def _run(self):
        try:
            import cv2
            import numpy as np
        except ImportError:
            self.skip("未安装 opencv-python")
            return

        from interface._camera import get_camera_indices
        from config.settings   import REPORT_DIR

        display_idx, _ = get_camera_indices(self)
        if display_idx is None:
            self.fail("未找到可用摄像头")
            return

        cap = cv2.VideoCapture(display_idx)
        if not cap.isOpened():
            self.fail(f"摄像头 /dev/video{display_idx} 无法打开")
            return
        cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        for _ in range(5):
            cap.read()

        # 准备保存图片的目录
        ts      = datetime.now().strftime("%Y%m%d_%H%M%S")
        out_dir = Path(REPORT_DIR) / "captures"
        out_dir.mkdir(parents=True, exist_ok=True)

        # 停 lightdm（_camera.py 检测时已停过，这里再确认一次）
        self.cmd("systemctl stop lightdm", timeout=15)
        time.sleep(0.5)

        results, all_pass = [], True

        try:
            for label, bgra, check_fn in _COLOR_TESTS:
                self.cmd(_FB_WRITE_SCRIPT.format(bgra=bgra), timeout=15)
                time.sleep(self.COLOR_WAIT_S)

                # 刷新缓冲取最新帧
                for _ in range(8):
                    cap.read()
                ret, frame = cap.read()
                if not ret or frame is None:
                    results.append(f"{label}:✗")
                    all_pass = False
                    continue

                # 保存图片并附加到报告
                fname = out_dir / f"hdmi_{ts}_{label}.png"
                cv2.imwrite(str(fname), frame)
                self.attach_image(fname)

                h, w  = frame.shape[:2]
                roi   = frame[h//4: 3*h//4, w//4: 3*w//4]
                b_avg = float(np.mean(roi[:, :, 0]))
                g_avg = float(np.mean(roi[:, :, 1]))
                r_avg = float(np.mean(roi[:, :, 2]))

                ok = check_fn(r_avg, g_avg, b_avg)
                results.append(f"{label}:{'✓' if ok else '✗'}")
                if not ok:
                    all_pass = False
        finally:
            cap.release()
            self.cmd("systemctl start lightdm", timeout=10)

        summary = "  ".join(results)
        if all_pass:
            self.pass_(summary)
        else:
            self.fail(summary)


def get_tests(board) -> list:
    return [
        HdmiStatusTest(board),
        HdmiScreenTest(board),
    ]
