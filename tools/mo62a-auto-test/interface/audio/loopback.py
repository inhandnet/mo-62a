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
  2. 同时播放 1kHz 正弦波 + 录音 3 秒
  3. FFT 分析录音，验证 1kHz 分量的 SNR ≥ 20 dB
  4. 将录音保存为 WAV 文件并附加到报告

硬件要求：
  - 测试环回线插入 3.5mm 耳机孔
  - TLV320AIC3106 声卡（Card 0）
"""

from __future__ import annotations

import struct
import time
from pathlib import Path

from config.settings import REPORT_DIR
from interface.base import TestCase

# ── 硬件常量 ─────────────────────────────────────────────────────────────────
_CARD          = 0
_DEVICE        = f"hw:{_CARD},0"
_RATE          = 48000
_CHANNELS      = 2
_TEST_FREQ     = 1000       # Hz
_PLAY_VOL      = 0.4        # GStreamer volume (40%)
_REC_SECONDS   = 3
_SNR_THRESHOLD = 20.0       # dB，低于此值判定失败

# mixer numid（TLV320AIC3106）
_M_HP_SW        = 37   # HP Playback Switch
_M_HP_VOL       = 36   # HP Playback Volume (0~9)
_M_HP_DAC_VOL   = 31   # HP DAC Playback Volume (0~118)
_M_HP_L_DACL1   = 86   # Left HP Mixer DACL1 Switch
_M_HP_R_DACR1   = 94   # Right HP Mixer DACR1 Switch
_M_PGA_VOL      = 48   # PGA Capture Volume (0~119)
_M_PGA_SW       = 49   # PGA Capture Switch
_M_PGA_L_LINE1L = 61   # Left PGA Mixer Line1L Switch
_M_PGA_L_MIC3L  = 64   # Left PGA Mixer Mic3L Switch (V1.1 悬空，须关闭)
_M_PGA_L_MIC3R  = 65   # Left PGA Mixer Mic3R Switch (V1.1 接 3.5mm MIC，须开启)
_M_PGA_R_LINE1R = 67   # Right PGA Mixer Line1R Switch
_M_PGA_R_MIC3L  = 70   # Right PGA Mixer Mic3L Switch (V1.1 悬空，须关闭)
_M_PGA_R_MIC3R  = 71   # Right PGA Mixer Mic3R Switch (V1.1 接 3.5mm MIC，须开启)


# ── mixer 工具函数 ────────────────────────────────────────────────────────────
def _amixer_get(test: TestCase, numid: int) -> str:
    """读取 mixer 控制当前值，返回 ':values=...' 部分的字符串。"""
    _, out, _ = test.cmd(f"amixer -c {_CARD} cget numid={numid} 2>/dev/null")
    for line in out.splitlines():
        if ": values=" in line:
            return line.strip()
    return ""


def _amixer_set(test: TestCase, numid: int, val: str) -> None:
    test.cmd(f"amixer -c {_CARD} cset numid={numid} {val} 2>/dev/null")


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
            self.fail("Card 0（TLV320AIC3106）未找到")
            return

        # ── 保存原始 mixer 值（测试结束后恢复）────────────────────────────────
        orig = {
            _M_PGA_L_LINE1L: _amixer_get(self, _M_PGA_L_LINE1L),
            _M_PGA_L_MIC3L:  _amixer_get(self, _M_PGA_L_MIC3L),
            _M_PGA_L_MIC3R:  _amixer_get(self, _M_PGA_L_MIC3R),
            _M_PGA_R_LINE1R: _amixer_get(self, _M_PGA_R_LINE1R),
            _M_PGA_R_MIC3L:  _amixer_get(self, _M_PGA_R_MIC3L),
            _M_PGA_R_MIC3R:  _amixer_get(self, _M_PGA_R_MIC3R),
            _M_PGA_VOL:      _amixer_get(self, _M_PGA_VOL),
            _M_PGA_SW:       _amixer_get(self, _M_PGA_SW),
        }

        try:
            self._do_loopback_test()
        finally:
            # ── 恢复 mixer ────────────────────────────────────────────────────
            for numid, saved in orig.items():
                if "values=" in saved:
                    val = saved.split("values=")[-1]
                    _amixer_set(self, numid, val)

    def _do_loopback_test(self):
        # ── 配置 mixer ────────────────────────────────────────────────────────
        # 播放路径：DAC → HP Mixer → HP 输出
        _amixer_set(self, _M_HP_SW,      "1,1")
        _amixer_set(self, _M_HP_VOL,     "7,7")
        _amixer_set(self, _M_HP_DAC_VOL, "70,70")
        _amixer_set(self, _M_HP_L_DACL1, "1")
        _amixer_set(self, _M_HP_R_DACR1, "1")
        # 录音路径：MIC3R(pin14) → PGA Mixer → ADC
        # V1.1 原理图：3.5mm SLEEVE → MIC3R；MIC3L 悬空，必须关闭避免引入噪声
        _amixer_set(self, _M_PGA_L_LINE1L, "0")   # 关闭 Line1L（串扰）
        _amixer_set(self, _M_PGA_R_LINE1R, "0")   # 关闭 Line1R（串扰）
        _amixer_set(self, _M_PGA_L_MIC3L,  "0")   # MIC3L 悬空，关闭
        _amixer_set(self, _M_PGA_R_MIC3L,  "0")   # MIC3L 悬空，关闭
        _amixer_set(self, _M_PGA_L_MIC3R,  "1")   # MIC3R → Left ADC
        _amixer_set(self, _M_PGA_R_MIC3R,  "1")   # MIC3R → Right ADC
        _amixer_set(self, _M_PGA_SW,       "1,1")
        _amixer_set(self, _M_PGA_VOL,      "45,45")  # ~22.5dB，不削波

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
            self.fail("GStreamer 录音命令失败")
            return

        # ── 下载录音并分析 ────────────────────────────────────────────────────
        try:
            raw = self.board.get_file("/tmp/loopback_rec.raw")
        except Exception as e:
            self.fail(f"下载录音文件失败: {e}")
            return

        if len(raw) < _RATE * 2:     # 少于 0.5 秒数据
            self.fail(f"录音数据过短: {len(raw)} 字节")
            return

        # S16LE stereo interleaved → 左声道 numpy 数组
        # 格式：L0_lo L0_hi R0_lo R0_hi L1_lo L1_hi ...
        # np.frombuffer + [::2] 正确提取左声道样本
        try:
            import numpy as np
            all_samples = np.frombuffer(raw, dtype=np.int16)
            left_samples = all_samples[::2]   # 偶数索引 = 左声道
        except ImportError:
            self.fail("需要 numpy（pip install numpy）")
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

        svg_str = _make_spectrum_svg(left_samples, _RATE)
        if svg_str:
            svg_path.write_text(svg_str, encoding="utf-8")
            self.attach_image(str(svg_path))

        # ── 判定结果 ──────────────────────────────────────────────────────────
        snr_str = f"{snr:.1f} dB" if snr is not None else "N/A"
        wav_rel = wav_path.name
        if passed:
            self.pass_(f"1kHz SNR {snr_str}  录音→{wav_rel}")
        else:
            self.fail(
                f"1kHz SNR {snr_str} < {_SNR_THRESHOLD} dB  "
                f"（请检查测试环回线是否正确插入）  录音→{wav_rel}"
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
            # 邻近噪声：200~3000Hz 范围，排除 1kHz ±100Hz
            mask = ((freqs >= 200) & (freqs < 900)) | ((freqs > 1100) & (freqs <= 3000))
            noise = np.max(fft[mask]) if mask.any() else 1.0
            return float(20 * np.log10(f1k_amp / (noise + 1e-10)))
        except Exception:
            return None
