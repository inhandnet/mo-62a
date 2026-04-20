#!/usr/bin/env python3
"""快速测试设备发现功能。直接运行即可：python test_discovery.py"""
import sys
import os
sys.path.insert(0, os.path.dirname(__file__))

from framework.discovery import discover_devices

print("正在扫描网络中的 MO-62A 设备...")
devices = discover_devices(timeout=3.0, retries=2)

if not devices:
    print("未发现任何设备。请确认：")
    print("  1. 板子已上电并连网")
    print("  2. mo-discover 服务正在运行")
    print("  3. 主机与板子在同一网段")
else:
    print(f"发现 {len(devices)} 台设备：\n")
    for d in devices:
        print(f"  主机名:    {d.get('hostname', '-')}")
        print(f"  IP 地址:   {d.get('ip', '-')}")
        print(f"  MAC 地址:  {d.get('mac', '-')}")
        print(f"  版本:      {d.get('version', '-')}")
        print(f"  编译日期:  {d.get('build_date', '-')}")
        print()
