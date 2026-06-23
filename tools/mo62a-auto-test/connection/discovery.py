"""UDP 广播设备发现 — 兼容 mo-discover 服务协议。

协议：向 255.255.255.255:47622 发送 b"MO62A_DISCOVER"，
设备返回 JSON：{"hostname":…, "ip":…, "mac":…, "version":…, "build_date":…}
"""

from __future__ import annotations

import json
import socket
import time

DISCOVER_PORT    = 47622
DISCOVER_MAGIC   = b"MO62A_DISCOVER"
DISCOVER_TIMEOUT = 1.0
DISCOVER_RETRIES = 1


def discover(
    timeout: float = DISCOVER_TIMEOUT,
    retries: int   = DISCOVER_RETRIES,
) -> list[dict]:
    """扫描局域网中的 MO-62A 设备，返回设备信息列表。

    默认只扫描 1 秒，避免用户长时间等待；未找到时可再次点击扫描。
    """
    found: dict[str, dict] = {}  # ip → info，用于去重

    for _ in range(retries):
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        try:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.settimeout(timeout)
            sock.bind(("", 0))
            sock.sendto(DISCOVER_MAGIC, ("255.255.255.255", DISCOVER_PORT))

            deadline = time.monotonic() + timeout
            while True:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    break
                sock.settimeout(remaining)
                try:
                    data, addr = sock.recvfrom(4096)
                    device = _parse(data, addr[0])
                    if device and device["ip"] not in found:
                        found[device["ip"]] = device
                except socket.timeout:
                    break
        except OSError:
            pass
        finally:
            sock.close()

    return list(found.values())


def _parse(data: bytes, sender_ip: str) -> dict | None:
    try:
        payload = json.loads(data.decode("utf-8"))
        if not isinstance(payload, dict):
            return None
        return {
            "hostname":   payload.get("hostname", ""),
            "ip":         payload.get("ip", sender_ip),
            "mac":        payload.get("mac", ""),
            "version":    payload.get("version", ""),
            "build_date": payload.get("build_date", ""),
        }
    except Exception:
        return None
