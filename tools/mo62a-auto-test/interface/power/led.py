"""双色 LED 测试 — 红色 LED / 绿色 LED 各一项

共用一次摄像头采集（基准帧+红帧+绿帧），避免重复控制硬件。
"""

from __future__ import annotations
import time
from datetime import datetime
from pathlib import Path
from interface.base import TestCase

DOMINANCE = 20    # 主色通道领先量阈值


# ── 共享采集器 ────────────────────────────────────────────────────────────────
class _LedScanner:
    """红/绿两个测试共用一次 LED+摄像头采集，避免重复停/启 led-status。"""

    def __init__(self, board):
        self._board   = board
        self._r_color = None   # (R, G, B) at LED-red position
        self._g_color = None   # (R, G, B) at LED-green position
        self._images: list[Path] = []
        self._scanned = False

    def scan(self, test: TestCase) -> bool:
        """执行一次完整采集，返回是否成功。后续调用直接返回缓存。"""
        if self._scanned:
            return self._r_color is not None

        try:
            import cv2
            import numpy as np
        except ImportError:
            return False

        from interface._camera import get_camera_indices
        from config.settings import REPORT_DIR

        _, led_idx = get_camera_indices(test)
        if led_idx is None:
            return False

        cap = cv2.VideoCapture(led_idx)
        if not cap.isOpened():
            return False
        cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        for _ in range(5):
            cap.read()

        ts      = datetime.now().strftime("%Y%m%d_%H%M%S")
        out_dir = Path(REPORT_DIR) / "captures"
        out_dir.mkdir(parents=True, exist_ok=True)

        test.cmd("systemctl stop led-status 2>/dev/null")
        test.cmd("echo none > /sys/class/leds/red/trigger 2>/dev/null")
        test.cmd("echo none > /sys/class/leds/green/trigger 2>/dev/null")

        def _set(r, g):
            test.cmd(f"echo {r} > /sys/class/leds/red/brightness")
            test.cmd(f"echo {g} > /sys/class/leds/green/brightness")

        def _flush_capture():
            for _ in range(8):
                cap.read()
            ret, f = cap.read()
            return f if ret else None

        def _detect_color(frame_on, frame_off, channel):
            diff = frame_on[:, :, channel].astype(np.int16) - frame_off[:, :, channel].astype(np.int16)
            diff = np.clip(diff, 0, 255).astype(np.uint8)
            diff = cv2.GaussianBlur(diff, (9, 9), 0)
            h, w = diff.shape
            mh, mw = int(h * 0.1), int(w * 0.1)
            diff[:mh, :] = 0; diff[h-mh:, :] = 0
            diff[:, :mw] = 0; diff[:, w-mw:] = 0
            _, _, _, (x, y) = cv2.minMaxLoc(diff)
            pad = 6
            y1, y2 = max(0, y-pad), min(h, y+pad)
            x1, x2 = max(0, x-pad), min(w, x+pad)
            roi = frame_on[y1:y2, x1:x2]
            return (float(np.mean(roi[:,:,2])),
                    float(np.mean(roi[:,:,1])),
                    float(np.mean(roi[:,:,0])))

        try:
            _set(0, 0); time.sleep(1.5); frame_off = _flush_capture()

            _set(1, 0); time.sleep(1.5); frame_red = _flush_capture()
            if frame_red is not None:
                p = out_dir / f"led_{ts}_R.png"
                cv2.imwrite(str(p), frame_red)
                self._images.append(p)

            _set(0, 1); time.sleep(1.5); frame_green = _flush_capture()
            if frame_green is not None:
                p = out_dir / f"led_{ts}_G.png"
                cv2.imwrite(str(p), frame_green)
                self._images.append(p)

            if frame_off is not None and frame_red is not None:
                self._r_color = _detect_color(frame_red, frame_off, channel=2)
            if frame_off is not None and frame_green is not None:
                self._g_color = _detect_color(frame_green, frame_off, channel=1)

        finally:
            _set(0, 0)
            test.cmd("systemctl start led-status 2>/dev/null")
            cap.release()
            self._scanned = True

        return self._r_color is not None

    @property
    def images(self): return list(self._images)


# ── 红色 LED ──────────────────────────────────────────────────────────────────
class RedLedTest(TestCase):
    category_key = "cat_display"
    name_key     = "tn_led_red"

    def __init__(self, board, scanner: _LedScanner):
        super().__init__(board)
        self._scanner = scanner

    def _run(self):
        try:
            import cv2
        except ImportError:
            self.skip("未安装 opencv-python")
            return

        if not self._scanner.scan(self):
            self.fail("摄像头采集失败")
            return

        for p in self._scanner.images:
            if "_R." in p.name:
                self.attach_image(p)

        color = self._scanner._r_color
        if color is None:
            self.fail("红灯帧采集失败")
            return

        r, g, b = color
        if r > g + DOMINANCE and r > b + DOMINANCE:
            self.pass_("✓")
        else:
            self.fail(f"✗ RGB({r:.0f},{g:.0f},{b:.0f})")


# ── 绿色 LED ──────────────────────────────────────────────────────────────────
class GreenLedTest(TestCase):
    category_key = "cat_display"
    name_key     = "tn_led_green"

    def __init__(self, board, scanner: _LedScanner):
        super().__init__(board)
        self._scanner = scanner

    def _run(self):
        try:
            import cv2
        except ImportError:
            self.skip("未安装 opencv-python")
            return

        if not self._scanner.scan(self):
            self.fail("摄像头采集失败")
            return

        for p in self._scanner.images:
            if "_G." in p.name:
                self.attach_image(p)

        color = self._scanner._g_color
        if color is None:
            self.fail("绿灯帧采集失败")
            return

        r, g, b = color
        if g > r + DOMINANCE and g > b + DOMINANCE:
            self.pass_("✓")
        else:
            self.fail(f"✗ RGB({r:.0f},{g:.0f},{b:.0f})")


def get_tests(board) -> list:
    scanner = _LedScanner(board)
    return [
        RedLedTest(board, scanner),
        GreenLedTest(board, scanner),
    ]
