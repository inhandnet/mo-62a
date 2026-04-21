"""存储测试模块：LPDDR4 内存完整性 + Micro SD 读写速度"""

import re
from tests.base import TestCase
from gui.i18n import t


_MIN_READ_MB  = 15   # MB/s
_MIN_WRITE_MB = 5    # MB/s


def _parse_speed_mbs(text: str) -> float | None:
    """从 dd 输出中提取速度，统一转换为 MB/s。"""
    m = re.search(r'([\d.]+)\s*GB/s', text)
    if m:
        return float(m.group(1)) * 1000
    m = re.search(r'([\d.]+)\s*MB/s', text)
    if m:
        return float(m.group(1))
    m = re.search(r'([\d.]+)\s*kB/s', text)
    if m:
        return float(m.group(1)) / 1000
    return None


class SdSpeedModeTest(TestCase):
    """查询 SD 卡协商的速率模式（SDR104 / HS200 等）及时钟频率。"""
    category_key = "cat_storage"
    name_key = "tn_sd_speed_mode"

    def _run(self):
        sudo_pw = getattr(self.board, "_password", "")

        # 优先从 debugfs ios 读取（需 sudo + debugfs 已挂载）
        rc, out, _ = self.cmd(
            f"echo '{sudo_pw}' | sudo -S cat /sys/kernel/debug/mmc1/ios 2>/dev/null",
            timeout=5,
        )
        if rc == 0 and out.strip():
            timing = ""
            clock_hz = ""
            for line in out.splitlines():
                line = line.strip()
                if line.startswith("timing spec:"):
                    timing = line.split(":", 1)[1].strip()
                elif line.startswith("clock:"):
                    parts = line.split()
                    if len(parts) >= 2:
                        try:
                            clock_hz = int(parts[1])
                        except ValueError:
                            pass
            if timing or clock_hz:
                clock_mhz = f"{int(clock_hz) // 1_000_000} MHz" if clock_hz else ""
                parts = []
                if timing:
                    parts.append(t("sd_mode_timing", timing))
                if clock_mhz:
                    parts.append(t("sd_mode_clock", clock_mhz))
                self.info("  ".join(parts))
                return

        # 回退：从内核日志中找 mmc1 速率信息
        rc2, out2, _ = self.cmd(
            "journalctl -k --no-pager 2>/dev/null | grep -i 'mmc1' | grep -iE 'mode|timing|speed|SDR|HS|DDR' | tail -3"
        )
        if rc2 == 0 and out2.strip():
            self.info(out2.strip().splitlines()[-1].strip())
            return

        self.info(t("sd_mode_unknown"))


class SdReadSpeedTest(TestCase):
    category_key = "cat_storage"
    name_key = "tn_sd_read_speed"

    def _run(self):
        sudo_pw = getattr(self.board, "_password", "")
        self.cmd(
            f"echo '{sudo_pw}' | sudo -S sh -c 'echo 3 > /proc/sys/vm/drop_caches' 2>/dev/null",
            timeout=5,
        )
        rc, out, err = self.cmd(
            f"echo '{sudo_pw}' | sudo -S dd if=/dev/mmcblk1 of=/dev/null bs=4M count=50 2>&1",
            timeout=60,
        )
        combined = (out + err).strip()
        if rc != 0:
            last = combined.splitlines()[-1] if combined else "unknown"
            self.fail(t("sd_read_fail", last))
            return

        speed = _parse_speed_mbs(combined)
        if speed is None:
            last = combined.splitlines()[-1] if combined else "no output"
            self.fail(t("sd_read_fail", last))
            return

        if speed < _MIN_READ_MB:
            self.fail(t("sd_read_slow", f"{speed:.1f}", _MIN_READ_MB))
        else:
            self.pass_(t("sd_read_ok", f"{speed:.1f}"))


class SdWriteSpeedTest(TestCase):
    category_key = "cat_storage"
    name_key = "tn_sd_write_speed"

    def _run(self):
        rc, out, err = self.cmd(
            "dd if=/dev/zero of=/tmp/sd_write_test bs=4M count=50 oflag=dsync 2>&1",
            timeout=120,
        )
        combined = (out + err).strip()
        self.cmd("rm -f /tmp/sd_write_test 2>/dev/null", timeout=5)

        if rc != 0:
            last = combined.splitlines()[-1] if combined else "unknown"
            self.fail(t("sd_write_fail", last))
            return

        speed = _parse_speed_mbs(combined)
        if speed is None:
            last = combined.splitlines()[-1] if combined else "no output"
            self.fail(t("sd_write_fail", last))
            return

        if speed < _MIN_WRITE_MB:
            self.fail(t("sd_write_slow", f"{speed:.1f}", _MIN_WRITE_MB))
        else:
            self.pass_(t("sd_write_ok", f"{speed:.1f}"))


class LpddrMemtesterTest(TestCase):
    """LPDDR4 内存完整性测试（memtester 32MB × 1轮，19 种模式）"""
    category_key = "cat_storage"
    name_key = "tn_lpddr_memtest"

    def _run(self):
        sudo_pw = getattr(self.board, "_password", "")
        rc, out, err = self.cmd(
            f"echo '{sudo_pw}' | sudo -S memtester 32M 1 2>&1",
            timeout=120,
        )
        combined = (out + err).strip()
        if rc != 0 or "FAIL" in combined:
            fail_lines = [l for l in combined.splitlines() if "FAIL" in l]
            self.fail(t("mem_memtester_fail", "; ".join(fail_lines[:3]) or combined[-200:]))
        else:
            self.pass_(t("mem_memtester_ok"))


_MIN_MEM_BW_MIBS = 1000   # MiB/s，MEMCPY 均值低于此值判 FAIL


class LpddrBandwidthTest(TestCase):
    """LPDDR4 内存带宽测试（mbw 256MB MEMCPY 均值）"""
    category_key = "cat_storage"
    name_key = "tn_lpddr_bandwidth"

    def _run(self):
        rc, out, err = self.cmd(
            "mbw -n 3 256 2>&1",
            timeout=30,
        )
        combined = (out + err).strip()
        if rc != 0:
            self.fail(t("mem_bw_fail", combined[-200:]))
            return

        # 解析 "AVG  Method: MEMCPY ... Copy: 1478.350 MiB/s"
        import re
        m = re.search(r'AVG\s+Method: MEMCPY.*?Copy:\s*([\d.]+)\s*MiB/s', combined)
        if not m:
            self.fail(t("mem_bw_parse_fail", combined[-200:]))
            return

        speed = float(m.group(1))
        if speed < _MIN_MEM_BW_MIBS:
            self.fail(t("mem_bw_slow", f"{speed:.0f}", _MIN_MEM_BW_MIBS))
        else:
            self.pass_(t("mem_bw_ok", f"{speed:.0f}"))


def get_tests(board, manual_confirm_fn=None, manual_input_fn=None, **kwargs):
    args = (board, manual_confirm_fn, manual_input_fn)
    return [
        LpddrMemtesterTest(*args),
        LpddrBandwidthTest(*args),
        SdSpeedModeTest(*args),
        SdReadSpeedTest(*args),
        SdWriteSpeedTest(*args),
    ]
