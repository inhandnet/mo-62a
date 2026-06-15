"""DDR 内存测试（容量 + mbw 带宽）"""

from __future__ import annotations
import re
from interface.base import TestCase


# ── DDR 容量 ──────────────────────────────────────────────────────────────────
class DdrCapacityTest(TestCase):
    category_key = "cat_storage"
    name_key     = "tn_ddr_capacity"

    def _run(self):
        # 从设备树 memory 节点读取物理 DDR 总量
        # reg 格式：每 16 字节一组（8字节基地址 + 8字节大小），大端序
        script = (
            "python3 -c \""
            "import struct, glob;"
            "total=0;"
            "[total:=total+struct.unpack('>Q',open(f,'rb').read()[i+8:i+16])[0]"
            " for f in glob.glob('/sys/firmware/devicetree/base/memory@*/reg')"
            " for i in range(0,len(open(f,'rb').read()),16)"
            " if i+16<=len(open(f,'rb').read())];"
            "print(total//(1024**3))"
            "\""
        )
        rc, out, _ = self.cmd(script)

        if rc == 0 and out.strip().isdigit() and int(out.strip()) > 0:
            self.info(f"{out.strip()} GB")
        else:
            # 回退：dmesg 方案
            rc2, out2, _ = self.cmd("dmesg | grep 'Memory:' | head -1")
            m = re.search(r'\d+K/(\d+)K\s+available', out2)
            if m:
                self.info(f"{round(int(m.group(1))/(1024**2))} GB")
            else:
                self.fail("无法读取 DDR 容量")


# ── DDR 带宽（mbw）───────────────────────────────────────────────────────────
class DdrBandwidthTest(TestCase):
    category_key = "cat_storage"
    name_key     = "tn_ddr_bandwidth"

    # 测试块大小（MB），不宜过大以免耗时过长
    BLOCK_MB = 256

    def _run(self):
        # 先确认 mbw 是否可用
        rc, _, _ = self.cmd("which mbw 2>/dev/null")
        if rc != 0:
            self.skip("mbw 未安装，请执行: sudo apt-get install -y mbw")
            return

        # mbw -n 3 256：跑 3 轮，每轮 256 MB
        # 输出包含 MEMCPY / DUMB / MCBLOCK 三种模式的带宽
        rc, out, _ = self.cmd(
            f"mbw -n 3 {self.BLOCK_MB} 2>/dev/null", timeout=120
        )
        if rc != 0 or not out.strip():
            self.fail("mbw 执行失败")
            return

        # 提取 MEMCPY 模式的平均带宽（最后一轮或 AVG 行）
        # 典型输出：AVG     Method: MEMCPY   Elapsed: 0.09   MiB: 256.00   Copy: 2844.1 MiB/s
        results = {}
        for line in out.splitlines():
            for method in ("MEMCPY", "DUMB", "MCBLOCK"):
                if method in line and "Copy:" in line:
                    m = re.search(r'Copy:\s+([\d.]+)\s+MiB/s', line)
                    if m:
                        results[method] = float(m.group(1))

        if "MEMCPY" not in results:
            self.fail("无法解析 mbw 输出")
            return

        self.info(f"{results['MEMCPY']:.0f} MB/s")


def get_tests(board) -> list:
    return [
        DdrCapacityTest(board),
        DdrBandwidthTest(board),
    ]
