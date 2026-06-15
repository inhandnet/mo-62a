"""USB 测试

测试项：
  1. USB Hub    — USB2514 Hub（VID:PID=0424:2514）枚举检测（INFO）
  2. USB 枚举   — Hub 下游外设数量检测（PASS/FAIL，期望 4 个）
  3. USB 读速   — 对每个 USB 块设备裸读 100MB，不依赖文件系统（INFO）
"""

from __future__ import annotations
import re
from interface.base import TestCase

HUB_VID_PID      = "0424:2514"
EXPECTED_STORAGE = 4
READ_MB          = 100   # 每个设备读取量


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
            self.fail("lsusb 命令失败")
            return
        for line in out.splitlines():
            if HUB_VID_PID in line:
                m = re.search(r'ID\s+\S+\s+(.+)', line)
                self.info((m.group(1).strip() if m else line.strip())[:60])
                return
        self.fail(f"未找到 USB2514 Hub（{HUB_VID_PID}）")


# ── USB 枚举（4 口全插）────────────────────────────────────────────────────────
class UsbPortTest(TestCase):
    category_key = "cat_usb"
    name_key     = "tn_usb_enum"

    def _run(self):
        rc, out, _ = self.cmd("lsusb 2>/dev/null")
        if rc != 0 or not out.strip():
            self.fail("lsusb 命令失败")
            return
        storage_lines = [
            l for l in out.splitlines()
            if l.strip() and not any(x in l.lower() for x in
               ['root hub', '1d6b:', '0424:2514', 'bluetooth', 'hub'])
        ]
        count = len(storage_lines)
        if count >= EXPECTED_STORAGE:
            self.pass_(f"{count} 个设备")
        elif count > 0:
            self.fail(f"{count} 个设备（期望 {EXPECTED_STORAGE}）")
        else:
            self.fail("未检测到任何 USB 外设")


# ── USB 读速（裸设备，不依赖文件系统）────────────────────────────────────────
class UsbReadTest(TestCase):
    """对每个 USB 块设备顺序读 READ_MB MB，汇报各设备读速。"""
    category_key = "cat_usb"
    name_key     = "tn_usb_read"

    def _run(self):
        devs = _usb_block_devices(self.board)
        if not devs:
            self.fail("未找到 USB 块设备")
            return

        results = []
        for dev in sorted(devs):
            # 先 drop_caches，避免缓存影响
            self.cmd("sh -c 'echo 3 > /proc/sys/vm/drop_caches'", timeout=5)
            rc, out, _ = self.cmd(
                f"dd if=/dev/{dev} of=/dev/null bs=1M count={READ_MB}"
                f" iflag=direct 2>&1",
                timeout=60,
            )
            speed = self._parse_dd(out)
            results.append(f"{dev}:{speed}" if speed else f"{dev}:失败")

        msg = "  ".join(results)
        if all("失败" not in r for r in results):
            self.info(msg)
        else:
            self.fail(msg)

    @staticmethod
    def _parse_dd(output: str) -> str:
        """从 dd 输出提取速率字符串，如 '45.3 MB/s'。"""
        m = re.search(r'([\d.]+\s*[MGK]B/s)', output)
        return m.group(1) if m else ""


def get_tests(board) -> list:
    return [
        UsbHubTest(board),
        UsbPortTest(board),
        UsbReadTest(board),
    ]
