"""SD 卡测试（容量 + 读速 + 写速）"""

from __future__ import annotations
import re
from config.i18n import t
from interface.base import TestCase

SD_DEV  = "/dev/mmcblk1"
SD_MOUNT = "/"          # rootfs 挂载点，用于容量和写速


# ── SD 卡容量 ─────────────────────────────────────────────────────────────────
class SdCardCapacityTest(TestCase):
    category_key = "cat_storage"
    name_key     = "tn_sd_capacity"

    def _run(self):
        rc, out, _ = self.cmd(f"df -h {SD_MOUNT} 2>/dev/null | tail -1")
        if rc != 0 or not out.strip():
            self.fail(t("msg_sd_capacity_fail"))
            return

        # df -h 输出：/dev/root  29G  5.6G  23G  21%  /
        parts = out.split()
        if len(parts) < 6:
            self.fail(t("msg_sd_df_format", out.strip()))
            return

        # 将 "29G" 格式转为 "29 GB"
        raw = parts[1].strip()
        if raw.endswith("G"):
            raw = raw[:-1] + " GB"
        self.info(raw)


# ── SD 卡读速 ─────────────────────────────────────────────────────────────────
class SdCardReadTest(TestCase):
    category_key = "cat_storage"
    name_key     = "tn_sd_read"

    READ_MB = 64   # 顺序读取块大小

    def _run(self):
        # drop_caches 确保不走 page cache，读到的是真实 SD 卡速率
        self.cmd(
            "sh -c 'echo 3 > /proc/sys/vm/drop_caches'", timeout=10
        )

        rc, out, _ = self.cmd(
            f"dd if={SD_DEV} of=/dev/null bs=1M count={self.READ_MB} 2>&1",
            timeout=120
        )
        if rc != 0:
            self.fail(t("msg_sd_read_fail", out.strip()))
            return

        speed = self._parse_dd_speed(out)
        if speed:
            self.info(speed)
        else:
            self.fail(t("msg_sd_parse_fail", out.strip()))

    @staticmethod
    def _parse_dd_speed(output: str) -> str:
        """从 dd stderr 中提取速率，如 '45.6 MB/s'。"""
        # dd 输出格式：134217728 bytes ... copied, 3.12 s, 43.0 MB/s
        m = re.search(r'([\d.]+\s*[MGK]B/s)', output)
        return m.group(1) if m else ""


# ── SD 卡写速 ─────────────────────────────────────────────────────────────────
class SdCardWriteTest(TestCase):
    category_key = "cat_storage"
    name_key     = "tn_sd_write"

    WRITE_MB = 16   # 写入块大小

    def _run(self):
        tmp_file = "/var/tmp/mo_sd_write_test"
        try:
            rc, out, _ = self.cmd(
                f"dd if=/dev/zero of={tmp_file} bs=1M count={self.WRITE_MB}"
                f" conv=fsync 2>&1",
                timeout=120
            )
            if rc != 0:
                self.fail(t("msg_sd_write_fail", out.strip()))
                return

            speed = SdCardReadTest._parse_dd_speed(out)
            if speed:
                self.info(speed)
            else:
                self.fail(t("msg_sd_parse_fail", out.strip()))
        finally:
            self.cmd(f"rm -f {tmp_file} 2>/dev/null")


def get_tests(board) -> list:
    return [
        SdCardCapacityTest(board),
        SdCardReadTest(board),
        SdCardWriteTest(board),
    ]
