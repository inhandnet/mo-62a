"""Network interface tests: Ethernet ping-gateway and Wi-Fi connect."""

import re

from tests.base import TestCase, TestResult
from gui.i18n import t


def _get_default_gateway(tc: TestCase) -> str | None:
    rc, out, _ = tc.cmd("ip route show default")
    if rc != 0:
        return None
    for line in out.splitlines():
        parts = line.split()
        if "via" in parts:
            return parts[parts.index("via") + 1]
    return None


def _ping_gateway(tc: TestCase, gw: str) -> tuple[bool, str]:
    rc, out, _ = tc.cmd(f"ping -c 3 -W 2 {gw}", timeout=15)
    match = re.search(r"(\d+)% packet loss", out)
    loss = match.group(0) if match else "unknown loss"
    return rc == 0, f"gateway {gw}: {loss}"


# ── Ethernet ────────────────────────────────────────────────────────────────
class EthernetPingTest(TestCase):
    category = "Network"
    name_key = "tn_ethernet_ping"

    def _run(self):
        gw = _get_default_gateway(self)
        if not gw:
            self.fail(t("net_no_gateway"))
            return
        ok, msg = _ping_gateway(self, gw)
        if ok:
            self.pass_(msg)
        else:
            self.fail(msg)


# ── Wi-Fi ───────────────────────────────────────────────────────────────────
class WiFiConnectTest(TestCase):
    category = "Network"
    name_key = "tn_wifi_connect"
    requires_manual = True

    def _run(self):
        # Scan
        rc, out, _ = self.cmd(
            "nmcli -t -f SSID,SIGNAL dev wifi list 2>/dev/null"
        )
        if rc != 0 or not out.strip():
            self.fail(t("net_scan_fail"))
            return

        ssids = []
        for line in out.splitlines():
            ssid = line.split(":")[0].strip()
            if ssid and ssid not in ssids:
                ssids.append(ssid)

        if not ssids:
            self.fail(t("net_no_ap"))
            return

        # Ask user: select SSID + enter password (combined dialog)
        result = self.manual_input(
            t("net_wifi_select", len(ssids)), choices=ssids
        )
        if result is None:
            self.skip(t("net_cancelled"))
            return

        parts = result.split("\n", 1)
        ssid = parts[0].strip()
        password = parts[1].strip() if len(parts) > 1 else ""

        if not ssid:
            self.fail(t("net_no_ssid"))
            return

        # Connect (needs root; use stored login password for sudo)
        sudo_pw = getattr(self.board, "_password", "")
        # 删除旧 profile，避免 key-mgmt 缺失导致的连接失败
        self.cmd(f"echo '{sudo_pw}' | sudo -S nmcli connection delete '{ssid}' 2>/dev/null", timeout=10)
        connect_cmd = (
            f"echo '{sudo_pw}' | sudo -S nmcli dev wifi connect '{ssid}' ifname wlan0"
            + (f" password '{password}'" if password else "")
        )
        rc, out, err = self.cmd(connect_cmd, timeout=30)
        combined = (out + err).strip()
        if rc != 0 or "error" in combined.lower():
            self.fail(t("net_connect_fail", combined))
            return

        # Ping gateway
        gw = _get_default_gateway(self)
        ping_msg = ""
        if gw:
            ok, ping_msg = _ping_gateway(self, gw)
            if not ok:
                ping_msg = f"connected but {ping_msg}"

        # Disconnect
        self.cmd(f"echo '{sudo_pw}' | sudo -S nmcli dev disconnect wlan0 2>/dev/null", timeout=10)

        self.pass_(t("net_wifi_ok", ssid, ping_msg).strip())


# ── BLE ─────────────────────────────────────────────────────────────────────
class BLEScanTest(TestCase):
    category = "Network"
    name_key = "tn_ble_scan"

    def _run(self):
        # Keep bluetoothctl alive via pipe so discovery isn't cancelled on exit
        rc, out, _ = self.cmd(
            "(echo 'scan on'; sleep 5; echo 'devices'; echo 'quit') | bluetoothctl 2>/dev/null",
            timeout=15,
        )
        lines = [l.strip() for l in out.splitlines() if l.strip().startswith("Device")]
        if not lines:
            self.fail(t("net_ble_none"))
            return
        self.pass_(t("net_ble_ok", len(lines)))


def get_tests(board, manual_confirm_fn=None, manual_input_fn=None, **kwargs):
    args = (board, manual_confirm_fn, manual_input_fn)
    return [
        EthernetPingTest(*args),
        WiFiConnectTest(*args),
        BLEScanTest(*args),
    ]
