"""以太网测试：速率、IP 地址、打流（iperf3）"""

from __future__ import annotations
import json
import platform
import re
import time
from pathlib import Path
from config.i18n import t
from config.settings import ROOT_DIR
from interface.base import TestCase


_DUR = 2   # iperf3 测试时长（秒）


def _eth_iface(board) -> str:
    """获取第一个非 lo 的以太网接口名。"""
    _, out, _ = board.run(
        "ip -o link show | awk -F': ' '{print $2}' | grep -v lo | head -1"
    )
    return out.strip()


def _find_local_iperf3() -> str | None:
    """查找本机可用的 iperf3 可执行文件路径。

    Windows 优先使用项目 bin/ 目录下的 iperf3.exe；
    Linux/macOS 使用系统 PATH 中的 iperf3。
    """
    system = platform.system().lower()
    if system == "windows":
        # 1) 优先项目自带
        bundled = ROOT_DIR / "bin" / "iperf3.exe"
        if bundled.exists():
            return str(bundled)
        # 2) 再尝试 PATH
        import shutil
        exe = shutil.which("iperf3.exe")
        if exe:
            return exe
    else:
        import shutil
        exe = shutil.which("iperf3")
        if exe:
            return exe
    return None


# ── 以太网速率 ────────────────────────────────────────────────────────────────
class EthernetSpeedTest(TestCase):
    category_key = "cat_network"
    name_key     = "tn_eth_speed"

    def _run(self):
        iface = _eth_iface(self.board)
        if not iface:
            self.fail(t("msg_eth_iface_missing"))
            return

        rc, out, _ = self.cmd(f"cat /sys/class/net/{iface}/speed 2>/dev/null")
        if rc != 0 or not out.strip().lstrip("-").isdigit():
            self.fail(t("msg_eth_speed_fail", iface))
            return

        speed = int(out.strip())
        if speed < 0:
            self.fail(t("msg_eth_link_down", iface))
            return

        unit = "Gbps" if speed >= 1000 else "Mbps"
        val  = speed // 1000 if speed >= 1000 else speed
        self.info(f"{val} {unit}")


# ── IP 地址 ───────────────────────────────────────────────────────────────────
class EthernetIPTest(TestCase):
    category_key = "cat_network"
    name_key     = "tn_eth_ip"

    def _run(self):
        iface = _eth_iface(self.board)
        if not iface:
            self.fail(t("msg_eth_iface_missing"))
            return

        rc, out, _ = self.cmd(
            f"ip -4 addr show {iface} | grep inet | awk '{{print $2}}' | head -1"
        )
        if rc != 0 or not out.strip():
            self.fail(t("msg_eth_ip_missing", iface))
            return

        self.info(out.strip())


# ── 打流测试（iperf3）────────────────────────────────────────────────────────
class EthernetIperfTest(TestCase):
    category_key = "cat_network"
    name_key     = "tn_eth_iperf"

    DURATION = _DUR

    def _run(self):
        # 检查设备上是否有 iperf3
        rc, _, _ = self.cmd("which iperf3 2>/dev/null")
        if rc != 0:
            self.skip(t("msg_iperf3_device_missing"))
            return

        # 检查本机是否有 iperf3（Windows 用 bin/iperf3.exe）
        local_iperf3 = _find_local_iperf3()
        if not local_iperf3:
            self.skip(t("msg_iperf3_host_missing"))
            return

        device_ip = self.board.host

        # 在设备上启动 iperf3 server（--one-off：一个客户端后自动退出）
        self.cmd("pkill -f 'iperf3 -s' 2>/dev/null; sleep 0.3")
        self.cmd("nohup iperf3 -s --one-off > /dev/null 2>&1 &")
        time.sleep(1.0)   # 等待 server 就绪

        # 本机作为 client 发起测试，-J 输出 JSON
        rc, out, err = self.local_cmd(
            f'"{local_iperf3}" -c {device_ip} -t {self.DURATION} -J',
            timeout=self.DURATION + 15,
        )

        # 清理设备上可能残留的 server
        self.cmd("pkill -f 'iperf3 -s' 2>/dev/null")

        if rc != 0 or not out.strip():
            self.fail(t("msg_iperf3_connect_fail", err.strip()[:80]))
            return

        try:
            data = json.loads(out)
            recv = data["end"]["sum_received"]["bits_per_second"] / 1e6
            self.info(f"{recv:.0f} Mbps")
        except (KeyError, json.JSONDecodeError) as e:
            self.fail(f"iperf3 结果解析失败: {e}")


def get_tests(board) -> list:
    return [
        EthernetSpeedTest(board),
        EthernetIPTest(board),
        EthernetIperfTest(board),
    ]
