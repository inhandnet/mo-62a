"""
I2C 总线测试模块
"""

from tests.base import TestCase, TestResult


class I2c1SiI9022Test(TestCase):
    category = "I2C 总线"
    name_key = "tn_i2c1_sii9022a"

    def _run(self):
        rc, out, err = self.cmd("i2cdetect -y 1 2>/dev/null")
        if "3b" not in out.lower():
            self.fail("I2C-1 上未检测到 SiI9022A（地址 0x3B）")
            return
        self.pass_("I2C-1 检测到 SiI9022A（0x3B）")


class I2c1AudioCodecTest(TestCase):
    category = "I2C 总线"
    name_key = "tn_i2c1_tlv320"

    def _run(self):
        rc, out, err = self.cmd("i2cdetect -y 1 2>/dev/null")
        if "1b" not in out.lower():
            self.fail("I2C-1 上未检测到 TLV320AIC3106（地址 0x1B）")
            return
        self.pass_("I2C-1 检测到 TLV320AIC3106（0x1B）")


class I2c1NoEepromTest(TestCase):
    category = "I2C 总线"
    name_key = "tn_i2c1_eeprom"

    def _run(self):
        rc, out, err = self.cmd("i2cdetect -y 1 2>/dev/null")
        if "50" in out.lower():
            self.fail(
                "I2C-1 上检测到地址 0x50（EEPROM 已移除，但仍有响应）——"
                "DDC bypass conflict risk"
            )
            return
        self.pass_("I2C-1 地址 0x50 无响应（EEPROM 已移除，无冲突）")


class I2c2ScanTest(TestCase):
    category = "I2C 总线"
    name_key = "tn_i2c2_scan"

    def _run(self):
        rc, out, err = self.cmd("i2cdetect -y 2 2>/dev/null")
        if rc != 0:
            self.fail(f"i2cdetect -y 2 失败（rc={rc}）")
            return
        self.pass_("I2C-2 总线扫描正常")


def get_tests(board, manual_confirm_fn=None):
    return [
        I2c1SiI9022Test(board, manual_confirm_fn),
        I2c1AudioCodecTest(board, manual_confirm_fn),
        I2c1NoEepromTest(board, manual_confirm_fn),
        I2c2ScanTest(board, manual_confirm_fn),
    ]
