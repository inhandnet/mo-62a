"""音频输出测试

通过 GStreamer 向音频设备输出 1kHz 正弦波，验证软件通路畅通。
音量固定为 20%（volume=0.2），避免过大。
不需要人工确认，GStreamer 无报错即为通过。

测试项：
  1. HDMI 音频   — SiI9022A I2S，Card 1（hw:1,0），采样率 48000 Hz
"""

from __future__ import annotations
from config.i18n import t
from interface.base import TestCase

# GStreamer 参数
_FREQ       = 1000    # 1kHz 正弦波
_VOLUME     = 0.2     # 20% 音量，不过大
_DURATION_S = 2       # 播放时长（秒）
# num-buffers = duration * sample_rate / buffer_size（默认 800 samples/buffer）
_BUFFERS    = int(_DURATION_S * 48000 / 800)


def _play_tone(test: TestCase, device: str) -> tuple[bool, str]:
    """向指定 ALSA 设备播放 1kHz 测试音，返回 (成功, 错误信息)。"""
    rc, out, err = test.cmd(
        f"gst-launch-1.0 audiotestsrc "
        f"wave=sine freq={_FREQ} volume={_VOLUME} num-buffers={_BUFFERS} ! "
        f"audio/x-raw,rate=48000,channels=2 ! "
        f"alsasink device={device} 2>&1",
        timeout=_DURATION_S + 10,
    )
    combined = (out + err).strip()
    return rc == 0, combined[:80] if combined else ""


# ── HDMI 音频 ─────────────────────────────────────────────────────────────────
class HdmiAudioTest(TestCase):
    category_key = "cat_audio"
    name_key     = "tn_hdmi_audio"

    def _run(self):
        # 确认 HDMI 已连接
        rc, status, _ = self.cmd(
            "cat /sys/class/drm/card*-HDMI*/status 2>/dev/null | head -1"
        )
        if status.strip() != "connected":
            self.skip(t("msg_hdmi_not_connected"))
            return

        # 确认 HDMI 声卡存在
        rc, out, _ = self.cmd("cat /proc/asound/cards 2>/dev/null")
        if "HDMI" not in out:
            self.fail(t("msg_hdmi_card_missing"))
            return

        ok, err = _play_tone(self, "hw:1,0")
        if ok:
            self.pass_(t("msg_hdmi_audio_play", _FREQ, int(_VOLUME*100), _DURATION_S))
        else:
            self.fail(t("msg_gst_play_generic_fail") if not err else t("msg_gst_play_fail", err))


def get_tests(board) -> list:
    return [HdmiAudioTest(board)]
