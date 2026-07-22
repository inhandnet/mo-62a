#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
mo_hdmi_capture.py — Windows 端 HDMI 采集方法（视频二维码 + 音频测频，供工厂产测框架集成）。

视频三步（配合 Mo62A: display hdmi detect）：
    find      find_capture_card()        -> 采集卡视频设备名
    capture   capture_frame(name)        -> 抓一帧 -> png
    decode    decode_qr(png)             -> 二维码内容

音频三步（配合 Mo62A: audio test hdmi）：
    find-audio    find_audio_device()    -> 采集卡音频设备名
    capture-audio capture_audio(name)    -> 录一段 -> wav
    detect-freq   detect_tone(wav)       -> (主频 Hz, SNR)；SNR 高=干净正弦

关键硬件事实：
  这类 USB HDMI 采集卡是 HDMI sink，且**只有在视频流被采集时才会解 HDMI 音频**。
  所以 capture_audio 默认「视频+音频同一会话打开」（自动带上视频设备），只保留音频；
  否则纯开音频口只会拿到采集卡底噪。视频侧 capture_frame 打开视频流即已激活。
  SSH 回填（把二维码内容 / 测得频率写回 display / audio 命令的 stdin）由产测框架自理。

依赖：ffmpeg.exe（PATH 或 --ffmpeg）；视频 pip install pyzbar pillow（cv2 兜底可选）；
      音频测频 pip install numpy。

命令行：
    python mo_hdmi_capture.py find
    python mo_hdmi_capture.py capture       "UGREEN 25854" [--out cap.png] [--seconds 6]
    python mo_hdmi_capture.py decode        cap.png
    python mo_hdmi_capture.py find-audio
    python mo_hdmi_capture.py capture-audio "数字音频接口 (UGREEN 25854)" [--out cap.wav] [--seconds 5] [--video "UGREEN 25854"]
    python mo_hdmi_capture.py detect-freq   cap.wav [--min-snr 15]
"""

import argparse
import os
import re
import subprocess
import sys

FFMPEG = "ffmpeg.exe"

# Windows 控制台默认非 UTF-8：强制 UTF-8 I/O，保证中文设备名（如“数字音频接口”）正确
try:
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
except Exception:
    pass


def _list_dshow(ffmpeg):
    """返回 (video_names, audio_names)。兼容新/旧版 ffmpeg 输出格式。"""
    p = subprocess.run(
        [ffmpeg, "-hide_banner", "-list_devices", "true", "-f", "dshow", "-i", "dummy"],
        capture_output=True, encoding="utf-8", errors="replace")
    vids, auds, section = [], [], None
    for line in (p.stderr or "").splitlines():
        m = re.search(r'"([^"]+)"\s*\((video|audio)\)', line)      # 新版
        if m:
            (vids if m.group(2) == "video" else auds).append(m.group(1))
            continue
        if "DirectShow video devices" in line:
            section = "v"; continue
        if "DirectShow audio devices" in line:
            section = "a"; continue
        m = re.search(r'"([^"]+)"', line)                          # 旧版
        if m and "Alternative name" not in line:
            (vids if section == "v" else auds if section == "a" else []).append(m.group(1))
    return list(dict.fromkeys(vids)), list(dict.fromkeys(auds))


def _pick(names, deny, kw):
    """从设备名列表里排除 deny、优先命中 kw，挑一个。"""
    if not names:
        return None
    if len(names) == 1:
        return names[0]
    real = [n for n in names if not any(k in n.lower() for k in deny)]
    if len(real) == 1:
        return real[0]
    for n in (real or names):
        if any(k in n.lower() for k in kw):
            return n
    return real[0] if real else names[0]


# ========== 视频（display hdmi detect）==========

def find_capture_card(ffmpeg=FFMPEG):
    """① 获取 HDMI 视频采集卡设备名；找不到返回 None。"""
    vids, _ = _list_dshow(ffmpeg)
    return _pick(vids,
                 deny=("todesk", "obs virtual", "virtual", "webcam",
                       "integrated camera", "droidcam", "manycam", "camera"),
                 kw=("capture", "hdmi", "fhd", "usb video", "grabber", "cam link",
                     "elgato", "avermedia", "macrosilicon", "ugreen", "ms2109",
                     "ms2130", "screen", "usb3"))


def capture_frame(name, out_png="mo_hdmi_cap.png", ffmpeg=FFMPEG):
    """② 抓一帧 -> out_png（需先 activate 使 HDMI 连上、Mo62A 输出）。返回路径或 None。"""
    cmd = [ffmpeg, "-hide_banner", "-loglevel", "error", "-f", "dshow",
           "-i", "video=%s" % name, "-frames:v", "1", "-y", out_png]
    subprocess.run(cmd, capture_output=True, text=True, errors="ignore")
    return out_png if os.path.exists(out_png) and os.path.getsize(out_png) > 0 else None


def decode_qr(png_path):
    """③ 解析 png 里的二维码，返回内容字符串；无则 None。pyzbar 首选，cv2 兜底。"""
    try:
        from PIL import Image
        from pyzbar.pyzbar import decode as zbar_decode
        for sym in zbar_decode(Image.open(png_path).convert("RGB")):
            try:
                return sym.data.decode("utf-8", "strict")
            except UnicodeDecodeError:
                return sym.data.decode("latin-1")
    except Exception:
        pass
    try:
        import cv2
        img = cv2.imread(png_path)
        if img is not None:
            data, _, _ = cv2.QRCodeDetector().detectAndDecode(img)
            if data:
                return data
    except Exception:
        pass
    return None


def activate_capture_card(name, ffmpeg=FFMPEG, seconds=3, video_size=None):
    """（可选）打开视频流若干秒纯激活（拉高 HPD，使 Mo62A 输出）。成功返回 True。"""
    pre = ["-f", "dshow"]
    if video_size:
        pre += ["-video_size", video_size]
    cmd = [ffmpeg, "-hide_banner", "-loglevel", "error"] + pre + \
          ["-i", "video=%s" % name, "-t", str(seconds), "-f", "null", "-"]
    return subprocess.run(cmd, capture_output=True, text=True,
                          errors="ignore").returncode == 0


# ========== 音频（audio test hdmi）==========

def find_audio_device(ffmpeg=FFMPEG):
    """① 获取 HDMI 音频采集卡设备名；找不到返回 None。"""
    _, auds = _list_dshow(ffmpeg)
    return _pick(auds,
                 deny=("todesk", "obs", "virtual", "立体声混音", "stereo mix",
                       "麦克风阵列", "microphone array"),
                 kw=("数字音频", "digital audio", "hdmi", "capture", "ugreen",
                     "25854", "usb"))


def capture_audio(audio_name, out_wav="mo_hdmi_audio.wav", ffmpeg=FFMPEG,
                  seconds=5, video_name=None):
    """② 录一段 HDMI 音频 WAV，返回路径或 None。

    采集卡只有在视频流被采集时才解 HDMI 音频 -> 默认「视频+音频同会话打开」：
    video_name 不给则自动查找采集卡视频设备；-vn 丢弃视频只留音频。
    """
    if video_name is None:
        video_name = find_capture_card(ffmpeg)
    if video_name:
        # 视频拉到 null(强制采集卡真正 streaming 视频，它才解 HDMI 音频) + 音频写 wav
        cmd = [ffmpeg, "-hide_banner", "-loglevel", "error", "-f", "dshow",
               "-i", "video=%s:audio=%s" % (video_name, audio_name),
               "-map", "0:v", "-t", str(seconds), "-f", "null", "-",
               "-map", "0:a", "-t", str(seconds), "-ac", "1", "-y", out_wav]
    else:
        cmd = [ffmpeg, "-hide_banner", "-loglevel", "error", "-f", "dshow",
               "-i", "audio=%s" % audio_name, "-t", str(seconds),
               "-ac", "1", "-y", out_wav]
    subprocess.run(cmd, capture_output=True, text=True, errors="ignore")
    return out_wav if os.path.exists(out_wav) and os.path.getsize(out_wav) > 100 else None


def detect_tone(wav_path):
    """③ FFT 测主频。返回 (freq_hz, snr_db)；数据无效返回 (None, None)。SNR 高=干净正弦。"""
    import wave
    try:
        import numpy as np
    except ImportError:
        raise RuntimeError("需要 numpy：pip install numpy")
    with wave.open(wav_path, "rb") as wf:
        rate, ch = wf.getframerate(), wf.getnchannels()
        raw = wf.readframes(wf.getnframes())
    x = np.frombuffer(raw, dtype=np.int16).astype(np.float32)
    if ch > 1:
        x = x[::ch]
    if len(x) < rate // 4:
        return None, None
    x = x - x.mean()
    n = len(x)
    fft = np.abs(np.fft.rfft(x * np.hanning(n)))
    freqs = np.fft.rfftfreq(n, 1.0 / rate)
    lo = freqs >= 100
    if not lo.any():
        return None, None
    idx = int(np.argmax(fft * lo))
    peak, f = float(fft[idx]), float(freqs[idx])
    band = np.abs(freqs - f) > 50
    noise = float(np.max(fft[band])) if band.any() else 1.0
    snr = float(20.0 * np.log10(peak / (noise + 1e-9)))
    return f, snr


# ========== CLI ==========

def main():
    ap = argparse.ArgumentParser(description="HDMI 采集方法（视频二维码 + 音频测频）")
    ap.add_argument("--ffmpeg", default=FFMPEG)
    sub = ap.add_subparsers(dest="cmd", required=True)

    sub.add_parser("find", help="① 获取视频采集卡名")
    pc = sub.add_parser("capture", help="② 抓一帧 -> png"); pc.add_argument("name"); pc.add_argument("--out", default="mo_hdmi_cap.png")
    pd = sub.add_parser("decode", help="③ 解析二维码 -> 内容"); pd.add_argument("png")
    pa = sub.add_parser("activate", help="（可选）纯激活视频流若干秒"); pa.add_argument("name"); pa.add_argument("--seconds", type=int, default=3)

    sub.add_parser("find-audio", help="① 获取音频采集卡名")
    pca = sub.add_parser("capture-audio", help="② 录一段 -> wav（视频+音频同开）"); pca.add_argument("name"); pca.add_argument("--out", default="mo_hdmi_audio.wav"); pca.add_argument("--seconds", type=int, default=5); pca.add_argument("--video", default=None, help="采集卡视频设备名(不给自动查找)")
    pdf = sub.add_parser("detect-freq", help="③ 测主频"); pdf.add_argument("wav"); pdf.add_argument("--min-snr", type=float, default=15.0)

    args = ap.parse_args()

    if args.cmd == "find":
        name = find_capture_card(args.ffmpeg); print(name or ""); sys.exit(0 if name else 1)
    if args.cmd == "capture":
        png = capture_frame(args.name, args.out, args.ffmpeg); print(png or ""); sys.exit(0 if png else 1)
    if args.cmd == "decode":
        c = decode_qr(args.png); print(c if c is not None else ""); sys.exit(0 if c else 1)
    if args.cmd == "activate":
        ok = activate_capture_card(args.name, args.ffmpeg, args.seconds); print("OK" if ok else "FAIL"); sys.exit(0 if ok else 1)

    if args.cmd == "find-audio":
        name = find_audio_device(args.ffmpeg); print(name or ""); sys.exit(0 if name else 1)
    if args.cmd == "capture-audio":
        wav = capture_audio(args.name, args.out, args.ffmpeg, args.seconds, video_name=args.video); print(wav or ""); sys.exit(0 if wav else 1)
    if args.cmd == "detect-freq":
        f, snr = detect_tone(args.wav)
        if f is None:
            print(""); print("detect: 无有效音频", file=sys.stderr); sys.exit(1)
        print("%d" % round(f))
        print("freq=%.1f Hz snr=%.1f dB sine=%s" % (f, snr, snr >= args.min_snr), file=sys.stderr)
        sys.exit(0 if snr >= args.min_snr else 2)


if __name__ == "__main__":
    main()
