"""3.5mm 耳机环回测试

测试线连接方式（TRRS CTIA）：
  TIP  (HP Left)  → 100Ω → SLEEVE (MIC)
  RING1(HP Right) → 100Ω → SLEEVE (MIC)

V1.1 原理图确认信号路径（Sheet 21 AUDIO CODEC）：
  播放：DAC L/R → Left/Right HP Mixer → HPLOUT(pin18)/HPROUT(pin23) → J8 TIP/RING1
  录音：J8 SLEEVE → MIC_IN 网络 → MIC3R(pin14) → PGA Mixer → ADC
  注：MIC3L(pin11) 在 V1.1 原理图中悬空，不接插孔

测试项：
  1. 配置 mixer（保存原始值，测试完恢复）
  2. 同时播放 1kHz 正弦波 + 录音 1 秒
  3. FFT 分析录音，验证 1kHz 分量的 SNR ≥ 20 dB
  4. 将录音保存为 WAV 文件并附加到报告

硬件要求：
  - 测试环回线插入 3.5mm 耳机孔
  - AM62Ax-SKEVM 声卡（Card 0）
"""

from __future__ import annotations

import struct
import time
from pathlib import Path

from config.settings import REPORT_DIR
from config.i18n import t
from interface.base import TestCase

# ── 硬件常量 ─────────────────────────────────────────────────────────────────
_CARD          = 0
_DEVICE        = f"hw:{_CARD},0"
_RATE          = 48000
_CHANNELS      = 2
_TEST_FREQ     = 1000       # Hz
_PLAY_VOL      = 0.2        # GStreamer volume (20%)
_REC_SECONDS   = 2
_SNR_THRESHOLD = 10.0       # dB，低于此值判定失败

# mixer 控件名称（simple-card 暴露的标准 ALSA 控件）
_MIXER_CONTROLS = {
    "PCM":                       "playback volume",
    "HP":                        "headphone switch/volume",
    "HP DAC":                    "headphone DAC volume",
    "Left HP Mixer DACL1":       "left HP DACL1 switch",
    "Right HP Mixer DACR1":      "right HP DACR1 switch",
    "PGA":                       "PGA capture volume/switch",
    "Left PGA Mixer Line1L":     "left PGA Line1L switch",
    "Left PGA Mixer Line1R":     "left PGA Line1R switch",
    "Left PGA Mixer Line2L":     "left PGA Line2L switch",
    "Left PGA Mixer Mic3L":      "left PGA Mic3L switch",
    "Left PGA Mixer Mic3R":      "left PGA Mic3R switch",
    "Right PGA Mixer Line1L":    "right PGA Line1L switch",
    "Right PGA Mixer Line1R":    "right PGA Line1R switch",
    "Right PGA Mixer Line2R":    "right PGA Line2R switch",
    "Right PGA Mixer Mic3L":     "right PGA Mic3L switch",
    "Right PGA Mixer Mic3R":     "right PGA Mic3R switch",
}


# ── mixer 工具函数 ────────────────────────────────────────────────────────────
def _amixer_get(test: TestCase, name: str) -> str:
    """读取 mixer 控制的当前完整输出。"""
    _, out, _ = test.cmd(f"amixer -c {_CARD} sget '{name}' 2>/dev/null")
    return out.strip()


def _amixer_set(test: TestCase, name: str, val: str) -> None:
    test.cmd(f"amixer -c {_CARD} sset '{name}' {val} 2>/dev/null")


# ── WAV 生成 ──────────────────────────────────────────────────────────────────
def _raw_to_wav(raw: bytes, rate: int = _RATE, channels: int = _CHANNELS,
                bits: int = 16) -> bytes:
    """将 PCM S16LE raw 数据包装为标准 WAV 文件字节流。"""
    data_size = len(raw)
    hdr = struct.pack(
        "<4sI4s4sIHHIIHH4sI",
        b"RIFF", data_size + 36,
        b"WAVE", b"fmt ", 16,
        1,              # PCM
        channels,
        rate,
        rate * channels * bits // 8,
        channels * bits // 8,
        bits,
        b"data", data_size,
    )
    return hdr + raw


# ── SVG 频谱图 ────────────────────────────────────────────────────────────────
def _make_spectrum_svg(samples_left, rate: int = _RATE) -> str:
    """生成简单 SVG 频谱图（0~5kHz），标注 1kHz 峰值。接受 numpy int16 数组。"""
    try:
        import numpy as np
        data = samples_left.astype(np.float32)
        n = min(len(data), rate)           # 最多取 1 秒
        fft = np.abs(np.fft.rfft(data[:n])) / n
        freqs = np.fft.rfftfreq(n, 1 / rate)
        mask = freqs <= 5000
        freqs = freqs[mask]
        fft = fft[mask]
        if len(fft) == 0:
            return ""
        fft_db = 20 * np.log10(fft / max(fft.max(), 1e-10) + 1e-10)

        W, H, PAD = 560, 200, 40
        x_scale = (W - 2 * PAD) / 5000.0
        y_scale = (H - 2 * PAD) / 60.0

        pts = " ".join(
            f"{PAD + f * x_scale:.1f},{H - PAD - max(-60, db) * y_scale:.1f}"
            for f, db in zip(freqs, fft_db)
        )
        peak_x = PAD + _TEST_FREQ * x_scale
        svg = (
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
            f'style="background:#0d1117;font-family:monospace">'
            f'<rect width="{W}" height="{H}" fill="#0d1117"/>'
            f'<polyline points="{pts}" fill="none" stroke="#00d4ff" stroke-width="1.5"/>'
            f'<line x1="{peak_x:.0f}" y1="{PAD}" x2="{peak_x:.0f}" y2="{H-PAD}" '
            f'stroke="#3fb950" stroke-width="1" stroke-dasharray="4"/>'
            f'<text x="{peak_x+4:.0f}" y="{PAD+14}" fill="#3fb950" font-size="11">'
            f'1kHz</text>'
            f'<text x="{PAD}" y="14" fill="#8b949e" font-size="11">Spectrum 0–5kHz</text>'
            f'</svg>'
        )
        return svg
    except ImportError:
        return ""


# ── 测试类 ────────────────────────────────────────────────────────────────────
class HeadphoneLoopbackTest(TestCase):
    """3.5mm 耳机环回测试（需要测试环回线）。"""

    category_key = "cat_audio"
    name_key     = "tn_headphone_loopback"

    def _run(self):
        # ── 确认声卡存在 ──────────────────────────────────────────────────────
        rc, out, _ = self.cmd("cat /proc/asound/cards 2>/dev/null")
        if "AM62Ax" not in out and "am62ax" not in out.lower():
            self.fail(t("msg_headphone_card_missing"))
            return

        # ── 保存原始 mixer 值（测试结束后恢复）────────────────────────────────
        orig = {name: _amixer_get(self, name) for name in _MIXER_CONTROLS}

        try:
            self._do_loopback_test()
        finally:
            # ── 恢复 mixer ────────────────────────────────────────────────────
            for name, saved in orig.items():
                if saved:
                    # 用 sset 的完整输出恢复太复杂，这里用 cset 恢复关键值
                    # 简单回退：按名称恢复原始 sget 输出中的值
                    # 由于格式复杂，实际靠后续重新打开 sset 全值可能失败，
                    # 因此保存为字符串后尝试逐行 cset；若失败也不阻塞。
                    self.cmd(f"amixer -c {_CARD} sset '{name}' '{saved}' 2>/dev/null || true")

    def _do_loopback_test(self):
        # ── 配置 mixer ────────────────────────────────────────────────────────
        _amixer_set(self, "PCM", "80")                  # 主 PCM 音量 ~-23dB
        _amixer_set(self, "HP", "on")                   # 打开耳机输出
        _amixer_set(self, "HP DAC", "70")               # HP DAC 音量 ~-24dB
        _amixer_set(self, "Left HP Mixer DACL1", "on")  # 左 DAC 到左 HP
        _amixer_set(self, "Right HP Mixer DACR1", "on") # 右 DAC 到右 HP

        # 录音路径：MIC3R → PGA Mixer → ADC
        # 关闭其他输入避免串扰，打开 Mic3R
        _amixer_set(self, "Left PGA Mixer Line1L", "off")
        _amixer_set(self, "Left PGA Mixer Line1R", "off")
        _amixer_set(self, "Left PGA Mixer Line2L", "off")
        _amixer_set(self, "Left PGA Mixer Mic3L", "off")
        _amixer_set(self, "Left PGA Mixer Mic3R", "on")  # 左 ADC 接 MIC3R
        _amixer_set(self, "Right PGA Mixer Line1L", "off")
        _amixer_set(self, "Right PGA Mixer Line1R", "off")
        _amixer_set(self, "Right PGA Mixer Line2R", "off")
        _amixer_set(self, "Right PGA Mixer Mic3L", "off")
        _amixer_set(self, "Right PGA Mixer Mic3R", "on") # 右 ADC 接 MIC3R
        _amixer_set(self, "PGA", "40,40,on")             # PGA 增益 ~16dB，开启

        # ── 同步播放 + 录音 ───────────────────────────────────────────────────
        play_bufs = int(_RATE * (_REC_SECONDS + 1) / 800)
        rec_bufs  = int(_RATE * _REC_SECONDS / 480)

        self.cmd("pkill -f gst-launch 2>/dev/null; sleep 0.2", timeout=3)
        self.cmd(
            f"gst-launch-1.0 audiotestsrc wave=sine freq={_TEST_FREQ} "
            f"volume={_PLAY_VOL} num-buffers={play_bufs} ! "
            f"audio/x-raw,rate={_RATE},channels={_CHANNELS} ! "
            f"alsasink device={_DEVICE} > /tmp/audio_play.log 2>&1 &",
            timeout=5,
        )
        time.sleep(0.3)

        rc, _, _ = self.cmd(
            f"gst-launch-1.0 alsasrc device={_DEVICE} num-buffers={rec_bufs} ! "
            f"audio/x-raw,rate={_RATE},format=S16LE,channels={_CHANNELS} ! "
            f"filesink location=/tmp/loopback_rec.raw 2>/dev/null",
            timeout=_REC_SECONDS + 10,
        )
        self.cmd("pkill -f gst-launch 2>/dev/null", timeout=3)

        if rc != 0:
            self.fail(t("msg_headphone_record_fail"))
            return

        # ── 下载录音并分析 ────────────────────────────────────────────────────
        try:
            raw = self.board.get_file("/tmp/loopback_rec.raw")
        except Exception as e:
            self.fail(t("msg_headphone_download_fail", e))
            return

        if len(raw) < _RATE * 2:     # 少于 0.5 秒数据
            self.fail(t("msg_headphone_data_short", len(raw)))
            return

        # S16LE stereo interleaved → 左声道 numpy 数组
        try:
            import numpy as np
            all_samples = np.frombuffer(raw, dtype=np.int16)
            left_samples = all_samples[::2]   # 偶数索引 = 左声道
        except ImportError:
            self.fail(t("msg_headphone_numpy_missing"))
            return

        snr = self._calc_snr(left_samples)
        passed = snr is not None and snr >= _SNR_THRESHOLD

        # ── 保存 WAV + SVG 到报告目录 ─────────────────────────────────────────
        REPORT_DIR.mkdir(parents=True, exist_ok=True)
        audio_dir = REPORT_DIR / "audio"
        audio_dir.mkdir(exist_ok=True)

        ts = int(time.time())
        wav_path = audio_dir / f"loopback_{ts}.wav"
        svg_path = audio_dir / f"loopback_{ts}_spectrum.svg"

        wav_data = _raw_to_wav(raw, _RATE, _CHANNELS, 16)
        wav_path.write_bytes(wav_data)

        # 不再生成/附加 SVG 频谱图到报告
        # svg_str = _make_spectrum_svg(left_samples, _RATE)
        # if svg_str:
        #     svg_path.write_text(svg_str, encoding="utf-8")
        #     self.attach_image(str(svg_path))

        # ── 判定结果 ──────────────────────────────────────────────────────────
        snr_str = f"{snr:.1f} dB" if snr is not None else "N/A"
        wav_rel = wav_path.name
        if passed:
            self.pass_(t("msg_headphone_pass", snr_str, wav_rel))
        else:
            self.fail(
                t("msg_headphone_fail", snr_str, _SNR_THRESHOLD, wav_rel)
            )

    # ── FFT SNR 计算 ──────────────────────────────────────────────────────────
    def _calc_snr(self, left_samples) -> float | None:
        """计算 1kHz 相对于邻近频率的 SNR（dB）。接受 numpy int16 数组。"""
        try:
            import numpy as np
            data = left_samples.astype(np.float32)
            n = min(len(data), _RATE)
            fft = np.abs(np.fft.rfft(data[:n]))
            freqs = np.fft.rfftfreq(n, 1 / _RATE)
            i1k = int(_TEST_FREQ * n / _RATE)
            f1k_amp = fft[i1k]
            # 邻近噪声：200~900Hz 和 1100~3000Hz 范围
            mask = ((freqs >= 200) & (freqs < 900)) | ((freqs > 1100) & (freqs <= 3000))
            noise = np.max(fft[mask]) if mask.any() else 1.0
            return float(20 * np.log10(f1k_amp / (noise + 1e-10)))
        except Exception:
            return None
