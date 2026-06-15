"""Wi-Fi 测试：热点扫描、信号质量

两个测试共用一次 iw scan 结果，避免重复扫描。
"""

from __future__ import annotations
import re
from interface.base import TestCase

_SCAN_TIMEOUT = 25


def _wlan_iface(board) -> str:
    _, out, _ = board.run(
        "ip -o link show | awk -F': ' '{print $2}' | grep -i wlan | head -1"
    )
    return out.strip()


# ── 共享扫描器 ────────────────────────────────────────────────────────────────
class _WifiScanner:
    """在两个测试实例间共享一次 iw scan 结果。"""

    def __init__(self, board):
        self._board  = board
        self._result = None   # None 表示尚未扫描

    def scan(self, test: TestCase) -> str | None:
        """执行扫描（只跑一次），返回原始输出；失败返回 None。"""
        if self._result is not None:
            return self._result

        iface = _wlan_iface(self._board)
        if not iface:
            self._result = ""
            return self._result

        test.cmd(f"ip link set {iface} up 2>/dev/null")

        _, out, _ = test.cmd(
            f"iw dev {iface} scan 2>/dev/null",
            timeout=_SCAN_TIMEOUT,
        )
        self._result = out
        self._iface  = iface
        return self._result

    @property
    def iface(self) -> str:
        return getattr(self, "_iface", "")


# ── WLAN 扫描 ─────────────────────────────────────────────────────────────────
class WiFiScanTest(TestCase):
    category_key = "cat_network"
    name_key     = "tn_wifi_scan"

    def __init__(self, board, scanner: _WifiScanner):
        super().__init__(board)
        self._scanner = scanner

    def _run(self):
        out = self._scanner.scan(self)
        if out is None or not self._scanner.iface:
            self.fail("未找到 Wi-Fi 接口")
            return

        count = len(re.findall(r'^BSS ', out, re.MULTILINE))
        self.info(f"发现 {count} 个热点")


# ── WLAN 信号 ─────────────────────────────────────────────────────────────────
class WiFiSignalTest(TestCase):
    category_key = "cat_network"
    name_key     = "tn_wifi_signal"

    def __init__(self, board, scanner: _WifiScanner):
        super().__init__(board)
        self._scanner = scanner

    def _run(self):
        out = self._scanner.scan(self)
        if out is None or not self._scanner.iface:
            self.fail("未找到 Wi-Fi 接口")
            return

        signals = [float(m) for m in re.findall(r'signal:\s*([-\d.]+)\s*dBm', out)]
        if not signals:
            self.fail("未解析到信号强度")
            return

        self.info(f"最强 {max(signals):.0f} dBm")


def get_tests(board) -> list:
    scanner = _WifiScanner(board)
    return [
        WiFiScanTest(board, scanner),
        WiFiSignalTest(board, scanner),
    ]
