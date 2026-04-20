"""
以太网测试模块
"""

import re

from tests.base import TestCase, TestResult


class EthInterfaceTest(TestCase):
    category = "以太网"
    name_key = "tn_eth0_status"

    def _run(self):
        if not self.assert_contains(
            "ip link show eth0 2>/dev/null",
            "UP",
            "eth0 接口未处于 UP 状态",
        ):
            return
        self.pass_("eth0 接口处于 UP 状态")


class IpAddressTest(TestCase):
    category = "以太网"
    name_key = "tn_eth0_ip"

    def _run(self):
        rc, out, err = self.cmd("ip -4 addr show eth0 2>/dev/null")
        if "inet" not in out:
            self.fail("eth0 未获取到 IPv4 地址")
            return
        # 提取 IP 地址
        match = re.search(r"inet\s+(\d+\.\d+\.\d+\.\d+/\d+)", out)
        ip_info = match.group(1) if match else "(解析失败)"
        self.pass_(f"eth0 IP 地址：{ip_info}")


class GatewayPingTest(TestCase):
    category = "以太网"
    name_key = "tn_gateway_ping"

    def _run(self):
        # 获取默认网关
        rc, out, err = self.cmd("ip route | awk '/default/{print $3}'")
        gateway = out.strip().splitlines()[0] if out.strip() else ""
        if not gateway:
            self.fail("未找到默认网关")
            return

        # Ping 网关，发送 5 包，等待 2 秒超时
        rc2, out2, err2 = self.cmd(
            f"ping -c 5 -W 2 {gateway}", timeout=20
        )
        if rc2 != 0:
            self.fail(f"ping 网关 {gateway} 失败（rc={rc2}）")
            return

        # 检查丢包率
        if "0% packet loss" in out2:
            self.pass_(f"网关 {gateway} 连通，丢包率 0%")
        else:
            # 提取丢包率信息
            loss_match = re.search(r"(\d+)% packet loss", out2)
            loss = loss_match.group(0) if loss_match else "未知丢包率"
            self.fail(f"ping 网关 {gateway} 存在丢包：{loss}")


class DnsTest(TestCase):
    category = "以太网"
    name_key = "tn_dns_resolution"

    def _run(self):
        rc, out, err = self.cmd("nslookup google.com 2>/dev/null")
        if rc == 0 or "Address" in out:
            self.pass_("DNS 解析 google.com 成功")
        else:
            self.fail(f"DNS 解析失败（rc={rc}）：{out.strip() or err.strip()}")


def get_tests(board, manual_confirm_fn=None):
    return [
        EthInterfaceTest(board, manual_confirm_fn),
        IpAddressTest(board, manual_confirm_fn),
        GatewayPingTest(board, manual_confirm_fn),
        DnsTest(board, manual_confirm_fn),
    ]
