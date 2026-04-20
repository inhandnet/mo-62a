"""
音频测试模块
"""

from tests.base import TestCase, TestResult


def _get_audio_card(board, keyword: str) -> str:
    """从 aplay -l 输出中获取包含关键字的声卡号（如 "0"、"1"）。"""
    rc, out, err = board.run("aplay -l 2>/dev/null")
    for line in out.splitlines():
        if keyword.lower() in line.lower() and line.startswith("card"):
            # 格式：card N: ...
            parts = line.split(":")
            if parts:
                card_num = parts[0].replace("card", "").strip()
                return card_num
    return ""


class AlsaDeviceTest(TestCase):
    category = "音频"
    name_key = "tn_alsa_device"

    def _run(self):
        rc, out, err = self.cmd("aplay -l 2>/dev/null")
        if rc != 0:
            self.fail(f"aplay -l 失败：{err.strip()}")
            return
        has_card0 = "card 0" in out
        has_card1 = "card 1" in out
        if not (has_card0 and has_card1):
            self.fail(
                f"未检测到至少两个 ALSA 声卡（card 0：{'有' if has_card0 else '无'}，"
                f"card 1：{'有' if has_card1 else '无'}）"
            )
            return
        self.pass_("检测到 card 0 和 card 1 两个声卡")


class PipeWireServiceTest(TestCase):
    category = "音频"
    name_key = "tn_pipewire_service"

    def _run(self):
        rc, out, err = self.cmd(
            "systemctl --user is-active pipewire 2>/dev/null || "
            "systemctl is-active pipewire"
        )
        if "active" not in out:
            self.fail(f"pipewire 服务未运行（输出：{out.strip()}）")
            return
        self.pass_("pipewire 服务正在运行")


class WirePlumberServiceTest(TestCase):
    category = "音频"
    name_key = "tn_wireplumber_service"

    def _run(self):
        rc, out, err = self.cmd(
            "systemctl --user is-active wireplumber 2>/dev/null || "
            "systemctl is-active wireplumber"
        )
        if "active" not in out:
            self.fail(f"wireplumber 服务未运行（输出：{out.strip()}）")
            return
        self.pass_("wireplumber 服务正在运行")


class HeadphonePlayTest(TestCase):
    category = "音频"
    name_key = "tn_headphone_output"
    requires_manual = True

    def _run(self):
        # 播放约 2 秒白噪声到模拟输出（hw:0,0）
        self.cmd(
            "aplay -D hw:0,0 /dev/urandom -f S16_LE -r 44100 -c 2 -d 2 2>/dev/null; true",
            timeout=10,
        )
        confirmed = self.manual_confirm(
            "请确认：3.5mm 耳机口有声音输出（白噪声约 2 秒）"
        )
        if not confirmed:
            return
        self.pass_("3.5mm 耳机输出听觉确认通过")


class HdmiAudioPlayTest(TestCase):
    category = "音频"
    name_key = "tn_hdmi_audio"
    requires_manual = True

    def _run(self):
        # 动态获取 HDMI 声卡号，优先找包含 "hdmi" 的卡
        card_num = _get_audio_card(self.board, "hdmi")
        if not card_num:
            # fallback：使用 card 1
            card_num = "1"

        self.cmd(
            f"aplay -D hw:{card_num},0 /dev/urandom -f S16_LE -r 44100 -c 2 -d 2 2>/dev/null; true",
            timeout=10,
        )
        confirmed = self.manual_confirm(
            f"请确认：HDMI 音频输出（声卡 {card_num}）有声音"
        )
        if not confirmed:
            return
        self.pass_(f"HDMI 音频输出（card {card_num}）听觉确认通过")


def get_tests(board, manual_confirm_fn=None):
    return [
        AlsaDeviceTest(board, manual_confirm_fn),
        PipeWireServiceTest(board, manual_confirm_fn),
        WirePlumberServiceTest(board, manual_confirm_fn),
        HeadphonePlayTest(board, manual_confirm_fn),
        HdmiAudioPlayTest(board, manual_confirm_fn),
    ]
