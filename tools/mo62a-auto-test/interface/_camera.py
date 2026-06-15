"""USB 摄像头自动分配

自动识别哪一路摄像头对准显示器、哪一路对准板载 LED，避免人工指定。

原理：
  显示器写入纯红色 → 两路摄像头各抓一帧 → 比较各帧 R 通道亮度
  R 显著更高的摄像头 = 显示器摄像头，另一路 = LED 摄像头

结果按 board.host 缓存，同一会话内只检测一次。
"""

from __future__ import annotations
import time

_cache: dict[str, tuple[int | None, int | None]] = {}



def get_camera_indices(test) -> tuple[int | None, int | None]:
    """返回 (display_cam_idx, led_cam_idx)。

    若只有一路摄像头，两个返回值相同。
    若无可用摄像头，返回 (None, None)。
    结果按 board.host 缓存。
    """
    key = test.board.host
    if key in _cache:
        return _cache[key]

    try:
        import cv2
    except ImportError:
        _cache[key] = (None, None)
        return _cache[key]

    cams = _list_cameras(cv2)
    if not cams:
        result = (None, None)          # 无摄像头，两类测试均 None
    elif len(cams) == 1:
        result = (cams[0], None)       # 1 路：只用于显示测试，LED 测试 SKIP
    else:
        result = _detect(test, cv2, cams)

    _cache[key] = result
    return result


def _list_cameras(cv2_mod=None, max_index: int = 8) -> list[int]:
    """列出每个物理 USB 摄像头的主流节点索引，跨平台。

    Linux：优先用 sysfs 按物理 USB 设备分组，取每组索引最小节点。
    Windows：OpenCV 每个摄像头只暴露一个索引，直接枚举可读的节点。
    """
    import platform
    if platform.system() == "Linux":
        cams = _list_cameras_by_v4l2()
        if cams:
            return cams
    # Windows / macOS / fallback
    return _list_cameras_by_open(cv2_mod, max_index)


def _list_cameras_by_v4l2() -> list[int]:
    """通过 sysfs 按物理 USB 设备分组，取每组索引最小的 video 节点。

    /sys/class/video4linux/videoN 的 realpath 包含 USB device 路径，
    同一物理摄像头的所有节点共享同一个父 USB 设备目录。
    """
    import glob, os

    groups: dict[str, list[int]] = {}   # usb_device_path → [video_idx, ...]

    for sysfs_path in glob.glob("/sys/class/video4linux/video*"):
        name = os.path.basename(sysfs_path)
        try:
            idx = int(name.replace("video", ""))
        except ValueError:
            continue
        try:
            real = os.path.realpath(sysfs_path)
            # 向上两级到 USB interface 目录（video4linux/videoN → uvcvideo → usb-interface）
            usb_iface = os.path.dirname(os.path.dirname(real))
            if usb_iface not in groups:
                groups[usb_iface] = []
            groups[usb_iface].append(idx)
        except Exception:
            continue

    if not groups:
        return []
    return sorted(min(v) for v in groups.values())


def _list_cameras_by_open(cv2_mod, max_index: int = 8) -> list[int]:
    """回退：逐个尝试打开，只保留能读到帧的节点。"""
    if cv2_mod is None:
        return []
    cams = []
    for idx in range(max_index):
        cap = cv2_mod.VideoCapture(idx)
        if not cap.isOpened():
            cap.release()
            continue
        cap.set(cv2_mod.CAP_PROP_BUFFERSIZE, 1)
        ret, frame = cap.read()
        cap.release()
        if ret and frame is not None:
            cams.append(idx)
    return cams


def _detect(test, cv2_mod, cams: list[int]) -> tuple[int, int]:
    """用双色 LED 作为校准源，识别哪路摄像头对准板子。

    原理：打开红灯前后各抓一帧，R 通道差值峰值最大的摄像头 = LED 摄像头。
    不依赖 HDMI 显示，HDMI 故障不影响分配结果。
    """
    import numpy as np

    # 接管 LED 控制
    test.cmd("systemctl stop led-status 2>/dev/null")
    test.cmd("echo none > /sys/class/leds/red/trigger 2>/dev/null")
    test.cmd("echo none > /sys/class/leds/green/trigger 2>/dev/null")

    def _capture_all() -> list:
        """从每路摄像头各抓一帧，返回 frame 或 None 列表。"""
        frames = []
        for idx in cams:
            cap = cv2_mod.VideoCapture(idx)
            if not cap.isOpened():
                frames.append(None)
                cap.release()
                continue
            cap.set(cv2_mod.CAP_PROP_BUFFERSIZE, 1)
            for _ in range(5):
                cap.read()
            ret, frame = cap.read()
            cap.release()
            frames.append(frame if ret else None)
        return frames

    try:
        # 基准帧（LED 全灭）
        test.cmd("echo 0 > /sys/class/leds/red/brightness")
        test.cmd("echo 0 > /sys/class/leds/green/brightness")
        time.sleep(1.2)
        frames_off = _capture_all()

        # 红灯亮帧
        test.cmd("echo 1 > /sys/class/leds/red/brightness")
        time.sleep(1.2)
        frames_on = _capture_all()

    finally:
        test.cmd("echo 0 > /sys/class/leds/red/brightness")
        test.cmd("systemctl start led-status 2>/dev/null")

    # 计算每路摄像头的 R 通道差值峰值（排除边缘）
    scores = []
    for idx, f_on, f_off in zip(cams, frames_on, frames_off):
        if f_on is None or f_off is None:
            scores.append((idx, 0.0))
            continue

        diff = f_on[:, :, 2].astype(np.int16) - f_off[:, :, 2].astype(np.int16)
        diff = np.clip(diff, 0, 255).astype(np.uint8)

        h, w = diff.shape
        mh, mw = int(h * 0.1), int(w * 0.1)
        diff[:mh, :] = 0;  diff[h-mh:, :] = 0
        diff[:, :mw] = 0;  diff[:, w-mw:] = 0

        peak = float(np.max(diff))
        scores.append((idx, peak))

    # 峰值最高的是 LED 摄像头，另一路是显示器摄像头
    scores.sort(key=lambda x: x[1], reverse=True)
    best_peak   = scores[0][1]
    second_peak = scores[1][1]

    # 置信度检查：最优摄像头的峰值须显著高于次优，且绝对值不能太低
    # 若置信度不足（LED 对摄像头几乎不可见），两者均返回 None
    MIN_PEAK       = 30    # LED 的 R 通道差值绝对峰值
    MIN_RATIO      = 2.0   # 最优 / 次优峰值比值

    if best_peak < MIN_PEAK:
        # LED 在两路摄像头中都几乎不可见，无法可靠分配
        return None, None

    if second_peak > 0 and (best_peak / second_peak) < MIN_RATIO:
        # 两路亮度差异不够大，可能两路都对准了 LED（或都没对准）
        return None, None

    led_idx     = scores[0][0]
    display_idx = scores[1][0]

    return display_idx, led_idx


def reset_cache(host: str | None = None) -> None:
    """清空缓存，用于重新检测（例如摄像头重新插拔后）。"""
    if host is None:
        _cache.clear()
    else:
        _cache.pop(host, None)
