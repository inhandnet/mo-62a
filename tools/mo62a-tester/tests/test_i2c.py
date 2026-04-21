"""I2C 总线测试模块

总线布局（来自 DTS）：
  i2c-0 (main_i2c0): PMIC TPS65931 @ 0x48, RTC PCF85363 @ 0x51
  i2c-1 (main_i2c1): TLV320AIC3106 @ 0x1B, SiI9022 HDMI @ 0x3B
  i2c-2 (main_i2c2): IMX219 摄像头 @ 0x10（需接模块，可选）

通过 /sys/bus/i2c/devices/{bus}-{addr:04x} 检测，无需 i2cdetect。
"""

from tests.base import TestCase
from gui.i18n import t


def _sysfs_path(bus: int, addr: int) -> str:
    return f"/sys/bus/i2c/devices/{bus}-{addr:04x}"


def _device_present(test_case, bus: int, addr: int) -> bool:
    rc, _, _ = test_case.cmd(f"test -d {_sysfs_path(bus, addr)}")
    return rc == 0


# ── I2C-0: PMIC + RTC ────────────────────────────────────────────────────────

class I2C0PmicTest(TestCase):
    category_key = "cat_i2c"
    name_key = "tn_i2c0_pmic"

    def _run(self):
        path = _sysfs_path(0, 0x48)
        if not _device_present(self, 0, 0x48):
            self.fail(t("i2c_pmic_missing", path))
        else:
            self.pass_(t("i2c_pmic_ok", path))


class I2C0RtcTest(TestCase):
    category_key = "cat_i2c"
    name_key = "tn_i2c0_rtc"

    def _run(self):
        path = _sysfs_path(0, 0x51)
        if not _device_present(self, 0, 0x51):
            self.fail(t("i2c_rtc_missing", path))
        else:
            self.pass_(t("i2c_rtc_ok", path))


# ── I2C-1: 音频编解码 + HDMI 桥 ──────────────────────────────────────────────

class I2C1TlvTest(TestCase):
    category_key = "cat_i2c"
    name_key = "tn_i2c1_tlv320"

    def _run(self):
        path = _sysfs_path(1, 0x1B)
        if not _device_present(self, 1, 0x1B):
            self.fail(t("i2c_tlv_missing", path))
        else:
            self.pass_(t("i2c_tlv_ok", path))


class I2C1SiiTest(TestCase):
    category_key = "cat_i2c"
    name_key = "tn_i2c1_sii9022a"

    def _run(self):
        path = _sysfs_path(1, 0x3B)
        if not _device_present(self, 1, 0x3B):
            self.fail(t("i2c_sii_missing", path))
        else:
            self.pass_(t("i2c_sii_ok", path))


# ── I2C-2: IMX219 摄像头（可选）──────────────────────────────────────────────

class I2C2Imx219Test(TestCase):
    """IMX219 未接时 skip，不算失败。"""
    category_key = "cat_i2c"
    name_key = "tn_i2c2_scan"

    def _run(self):
        path = _sysfs_path(2, 0x10)
        if not _device_present(self, 2, 0x10):
            self.skip(t("i2c_imx219_skip", path))
        else:
            self.pass_(t("i2c_imx219_ok", path))


def get_tests(board, manual_confirm_fn=None, manual_input_fn=None, **kwargs):
    args = (board, manual_confirm_fn, manual_input_fn)
    return [
        I2C0PmicTest(*args),
        I2C0RtcTest(*args),
        I2C1TlvTest(*args),
        I2C1SiiTest(*args),
        I2C2Imx219Test(*args),
    ]
