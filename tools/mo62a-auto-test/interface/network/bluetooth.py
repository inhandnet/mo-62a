"""蓝牙测试：BLE 扫描、BLE 信号质量

两个测试共用一次 btmgmt find 结果，避免重复扫描。

btmgmt find 输出格式：
  hci0 dev_found: AA:BB:CC:DD:EE:FF type LE Random rssi -50 flags 0x0004
"""

from __future__ import annotations
import re
from config.i18n import t
from interface.base import TestCase

SCAN_SEC = 3


# ── 共享扫描器 ────────────────────────────────────────────────────────────────
class _BLEScanner:
    """在两个测试实例间共享一次 btmgmt find 结果。"""

    def __init__(self, board):
        self._board  = board
        self._macs   = None   # None 表示尚未扫描
        self._rssies = None

    def scan(self, test: TestCase) -> bool:
        """执行扫描（只跑一次）。返回是否成功找到控制器。"""
        if self._macs is not None:
            return True

        rc, _, _ = test.cmd("hciconfig hci0 2>/dev/null | grep -q hci0")
        if rc != 0:
            self._macs   = []
            self._rssies = []
            return False

        test.cmd("hciconfig hci0 up 2>/dev/null")
        test.cmd("btmgmt stop-find 2>/dev/null; sleep 0.5")

        _, out, _ = test.cmd(
            f"timeout {SCAN_SEC} btmgmt find 2>/dev/null || true",
            timeout=SCAN_SEC + 5,
        )

        macs, rssies = [], []
        for line in out.splitlines():
            if "dev_found" not in line:
                continue
            if "type LE" not in line and "type le" not in line.lower():
                continue
            m = re.search(r'([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}', line)
            if m:
                macs.append(m.group(0))
            r = re.search(r'rssi\s+(-?\d+)', line)
            if r:
                rssies.append(int(r.group(1)))

        self._macs   = macs
        self._rssies = rssies
        return True

    @property
    def macs(self) -> list[str]:
        return self._macs or []

    @property
    def rssies(self) -> list[int]:
        return self._rssies or []


# ── BLE 扫描 ──────────────────────────────────────────────────────────────────
class BluetoothScanTest(TestCase):
    category_key = "cat_network"
    name_key     = "tn_bt_scan"

    def __init__(self, board, scanner: _BLEScanner):
        super().__init__(board)
        self._scanner = scanner

    def _run(self):
        ok = self._scanner.scan(self)
        if not ok:
            self.fail(t("msg_bt_ctrl_missing"))
            return
        self.info(t("msg_bt_count", len(set(self._scanner.macs))))


# ── BLE 信号 ──────────────────────────────────────────────────────────────────
class BluetoothSignalTest(TestCase):
    category_key = "cat_network"
    name_key     = "tn_bt_signal"

    def __init__(self, board, scanner: _BLEScanner):
        super().__init__(board)
        self._scanner = scanner

    def _run(self):
        ok = self._scanner.scan(self)
        if not ok:
            self.fail(t("msg_bt_ctrl_missing"))
            return

        rssies = self._scanner.rssies
        if not rssies:
            self.info(t("msg_bt_no_rssi"))
            return

        self.info(t("msg_bt_strongest", max(rssies)))


def get_tests(board) -> list:
    scanner = _BLEScanner(board)
    return [
        BluetoothScanTest(board, scanner),
        BluetoothSignalTest(board, scanner),
    ]
