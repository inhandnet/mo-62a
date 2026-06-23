"""USB 测试

测试项：
  1. USB Hub    — USB2514 Hub（VID:PID=0424:2514）枚举检测（INFO）
  2. USB 枚举   — Hub 下游外设数量检测（PASS/FAIL，期望 4 个）
  3. USB 读速   — 对每个 USB 块设备裸读 100MB，不依赖文件系统（INFO）
"""

from __future__ import annotations
import re
from config.i18n import t
from interface.base import TestCase

HUB_VID_PID      = "0424:2514"
EXPECTED_STORAGE = 4


# ── 工具：找所有 USB 块设备（磁盘级，非分区）─────────────────────────────────
def _usb_block_devices(board) -> list[str]:
    """返回 ['sda','sdb',...] 形式的 USB 块设备列表。"""
    _, out, _ = board.run(
        "lsblk -d -o NAME,TRAN --noheadings 2>/dev/null | awk '$2==\"usb\"{print $1}'"
    )
    return [d.strip() for d in out.splitlines() if d.strip()]


# ── USB Hub 检测 ──────────────────────────────────────────────────────────────
class UsbHubTest(TestCase):
    category_key = "cat_usb"
    name_key     = "tn_usb_hub"

    def _run(self):
        rc, out, _ = self.cmd("lsusb 2>/dev/null")
        if rc != 0 or not out.strip():
            self.info(t("msg_usb_lsusb_missing"))
            return
        for line in out.splitlines():
            if HUB_VID_PID in line:
                m = re.search(r'ID\s+(\S+)', line)
                self.info(m.group(1) if m else line.strip()[:20])
                return
        self.info(t("msg_usb_hub_not_found", HUB_VID_PID))


# ── USB 枚举（4 口全插）────────────────────────────────────────────────────────
class UsbPortTest(TestCase):
    category_key = "cat_usb"
    name_key     = "tn_usb_enum"

    def _run(self):
        devs = _usb_block_devices(self.board)
        count = len(devs)
        if count >= EXPECTED_STORAGE:
            self.pass_(t("msg_usb_count", count))
        elif count > 0:
            self.fail(t("msg_usb_count_expected", count, EXPECTED_STORAGE))
        else:
            self.fail(t("msg_usb_none"))


# ── USB 速率（测试 USB Hub 总带宽）────────────────────────────────────────
class UsbReadTest(TestCase):
    """对所有 USB 块设备并发读，计算 USB Hub 总带宽。

    4 个 U 盘同时读取，报告各设备速率及总带宽。利用多核 A53 并行处理，
    主要验证 USB Hub 的整体吞吐能力。
    """
    category_key = "cat_usb"
    name_key     = "tn_usb_read"

    READ_MB = 16   # 每个设备读取量

    def _run(self):
        devs = _usb_block_devices(self.board)
        if not devs:
            self.fail(t("msg_usb_block_missing"))
            return

        # 清缓存一次即可
        self.cmd("sh -c 'echo 3 > /proc/sys/vm/drop_caches'", timeout=5)

        # 为每个设备生成独立输出文件，并发后台 dd
        cmds = []
        for dev in sorted(devs):
            cmds.append(
                f"dd if=/dev/{dev} of=/dev/null bs=1M count={self.READ_MB} "
                f"iflag=direct 2>/tmp/dd_{dev}.log"
            )
        script = "set -m; " + " & ".join(f"({c})" for c in cmds) + "; wait"

        rc, _, _ = self.cmd(script, timeout=30)

        total_mbps = 0.0
        results = []
        for dev in sorted(devs):
            rc2, out, _ = self.cmd(f"cat /tmp/dd_{dev}.log 2>/dev/null")
            speed = self._parse_dd(out)
            if speed:
                results.append(f"{dev}:{speed}")
                total_mbps += self._speed_to_mbps(speed)
            else:
                results.append(t("msg_usb_dev_fail", dev))
            self.cmd(f"rm -f /tmp/dd_{dev}.log 2>/dev/null")

        msg = "  ".join(results) + f"  |  {t('msg_usb_total', f'{total_mbps:.1f}')}"
        if all(t("msg_usb_fail_short") not in r for r in results):
            self.info(msg)
        else:
            self.fail(msg)

    @staticmethod
    def _parse_dd(output: str) -> str:
        """从 dd 输出提取速率字符串，如 '45.3 MB/s'。"""
        m = re.search(r'([\d.]+\s*[MGK]B/s)', output)
        return m.group(1) if m else ""

    @staticmethod
    def _speed_to_mbps(speed: str) -> float:
        """把 dd 速率字符串转换为 MB/s 数值。"""
        m = re.search(r'([\d.]+)\s*([MGK])B/s', speed)
        if not m:
            return 0.0
        val = float(m.group(1))
        unit = m.group(2)
        if unit == "G":
            return val * 1024
        if unit == "M":
            return val
        if unit == "K":
            return val / 1024
        return 0.0


def get_tests(board) -> list:
    return [
        UsbHubTest(board),
        UsbPortTest(board),
        UsbReadTest(board),
    ]
