#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
pc_hdmi_capture.py — Windows 端 HDMI 采集回传示例。

配合 Mo62A 的工厂命令：
    display hdmi detect [expected_qr]
    audio test hdmi

流程：
  1. 调用 ffmpeg(dshow) 从 HDMI 采集卡抓取一帧视频 -> .jpg
  2. 调用 ffmpeg(dshow) 从 HDMI 采集卡录制音频 -> .wav
  3. 通过 scp 把 .jpg/.wav 回传到 Mo62A 固定路径

Windows 前提：
  - 安装 ffmpeg.exe 并加到 PATH（或同一目录）
  - 安装 OpenSSH Client（Windows 10/11 自带，用于 scp）
  - 采集卡驱动已安装，且在设备管理器中能看到：
      * 视频："USB Video Device" 或类似
      * 音频："USB Digital Audio" 或类似

用法：
    python pc_hdmi_capture.py --host Mo62A.local --video-only
    python pc_hdmi_capture.py --host Mo62A.local --audio-only
    python pc_hdmi_capture.py --host Mo62A.local
"""

import argparse
import os
import subprocess
import sys
import tempfile
import time

FFMPEG = "ffmpeg.exe"
SCP = "scp.exe"          # Windows 自带 OpenSSH 的 scp
SSH_USER = "debian"

# 采集卡 dshow 设备名；用下面命令查看实际名称后替换：
#   ffmpeg.exe -list_devices true -f dshow -i dummy
VIDEO_DEV = "USB Video"      # 常见名："USB Video", "HDMI Capture", ...
AUDIO_DEV = "Digital Audio Interface (USB Audio)"   # 常见名，按实际替换

REMOTE_VIDEO = "/tmp/mo_hdmi_cap.png"
REMOTE_AUDIO = "/tmp/mo_hdmi_audio.wav"


def _run(cmd, timeout=30):
    print(">>>", " ".join(cmd))
    r = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                       text=True, timeout=timeout)
    if r.returncode != 0:
        print(r.stderr[-800:], file=sys.stderr)
    return r.returncode == 0


def _scp(local, host, remote):
    dst = f"{SSH_USER}@{host}:{remote}"
    return _run([SCP, "-o", "StrictHostKeyChecking=no", "-o", "UserKnownHostsFile=/dev/null",
                 local, dst], timeout=30)


def capture_video(out_png):
    """用 ffmpeg dshow 抓一帧视频。"""
    cmd = [
        FFMPEG, "-y", "-hide_banner", "-loglevel", "error",
        "-f", "dshow", "-i", f"video={VIDEO_DEV}",
        "-vframes", "1", out_png,
    ]
    return _run(cmd, timeout=15)


def capture_audio(out_wav, seconds=2):
    """用 ffmpeg dshow 录 N 秒音频。"""
    cmd = [
        FFMPEG, "-y", "-hide_banner", "-loglevel", "error",
        "-f", "dshow", "-i", f"audio={AUDIO_DEV}",
        "-t", str(seconds),
        "-acodec", "pcm_s16le", "-ar", "48000", "-ac", "2",
        out_wav,
    ]
    return _run(cmd, timeout=seconds + 15)


def main():
    p = argparse.ArgumentParser(description="HDMI capture agent for Mo62A factory test")
    p.add_argument("--host", default="Mo62A.local", help="Mo62A hostname/IP")
    p.add_argument("--video-only", action="store_true", help="只回传视频")
    p.add_argument("--audio-only", action="store_true", help="只回传音频")
    args = p.parse_args()

    tmpdir = tempfile.mkdtemp(prefix="mo_hdmi_")
    ok = True

    if not args.audio_only:
        png = os.path.join(tmpdir, "mo_hdmi_cap.png")
        print("[video] 采集 HDMI 画面...")
        if not capture_video(png):
            print("[video] 采集失败", file=sys.stderr)
            ok = False
        else:
            print("[video] 回传 %s -> %s:%s" % (png, args.host, REMOTE_VIDEO))
            if not _scp(png, args.host, REMOTE_VIDEO):
                ok = False

    if not args.video_only:
        wav = os.path.join(tmpdir, "mo_hdmi_audio.wav")
        print("[audio] 采集 HDMI 音频...")
        if not capture_audio(wav, seconds=2):
            print("[audio] 采集失败", file=sys.stderr)
            ok = False
        else:
            print("[audio] 回传 %s -> %s:%s" % (wav, args.host, REMOTE_AUDIO))
            if not _scp(wav, args.host, REMOTE_AUDIO):
                ok = False

    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
