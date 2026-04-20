"""
UDP 广播设备发现，用于自动找到局域网中的 MO-62A 板子。
"""

import json
import socket
import time
from typing import Optional

from config import DISCOVER_MAGIC, DISCOVER_PORT, DISCOVER_TIMEOUT, DISCOVER_RETRIES


def discover_devices(
    timeout: float = DISCOVER_TIMEOUT,
    retries: int = DISCOVER_RETRIES,
) -> list[dict]:
    """通过 UDP 广播发现局域网中的 MO-62A 设备。

    每次重试使用新的 socket，合并所有轮次的响应，按 ip 去重。

    Args:
        timeout: 每轮等待响应的秒数
        retries: 重试轮次数

    Returns:
        设备列表，每个元素为 dict：
        {
            "hostname": str,
            "ip": str,
            "mac": str,
            "version": str,
            "build_date": str,
        }
        若无设备响应则返回空列表。
    """
    found: dict[str, dict] = {}  # ip -> device_info，用于去重

    for attempt in range(retries):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.settimeout(timeout)
            sock.bind(("", 0))

            # 发送广播
            sock.sendto(DISCOVER_MAGIC, ("255.255.255.255", DISCOVER_PORT))

            deadline = time.monotonic() + timeout
            while True:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    break
                sock.settimeout(remaining)
                try:
                    data, addr = sock.recvfrom(4096)
                except socket.timeout:
                    break

                # 尝试解析 JSON 响应
                device = _parse_response(data, addr[0])
                if device is not None:
                    ip = device["ip"]
                    if ip not in found:
                        found[ip] = device

        except OSError:
            # 网络不可用等情况，静默跳过
            pass
        finally:
            sock.close()

    return list(found.values())


def _parse_response(data: bytes, sender_ip: str) -> Optional[dict]:
    """解析设备响应报文。

    Args:
        data: 原始 UDP 数据
        sender_ip: 发送方 IP（用作 fallback）

    Returns:
        设备 dict 或 None（解析失败时）
    """
    try:
        payload = json.loads(data.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError):
        return None

    if not isinstance(payload, dict):
        return None

    device = {
        "hostname": payload.get("hostname", ""),
        "ip": payload.get("ip", sender_ip),
        "mac": payload.get("mac", ""),
        "version": payload.get("version", ""),
        "build_date": payload.get("build_date", ""),
    }
    return device
