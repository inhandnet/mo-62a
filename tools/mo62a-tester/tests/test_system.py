"""系统基础信息模块（仅采集展示，不做判定）"""

from tests.base import TestCase, TestResult
from gui.i18n import t


class _InfoTest(TestCase):
    """基类：命令成功 → INFO，失败 → FAIL。"""
    _cmd: str = ""

    def _run(self):
        rc, out, err = self.cmd(self._cmd)
        if rc != 0 and not out.strip():
            self.fail(err.strip() or "command failed")
            return
        self.info(self._fmt(out.strip()))

    def _fmt(self, out: str) -> str:
        return out.splitlines()[0] if out else t("sys_empty")


# ── 固件版本 ────────────────────────────────────────────────────────────────
class FirmwareVersionTest(_InfoTest):
    category_key = "cat_system"
    name_key = "tn_firmware_version"
    _cmd = "mo-version 2>/dev/null | head -2 | tr '\\n' '  ' || echo unknown"


# ── 内核版本 ────────────────────────────────────────────────────────────────
class KernelVersionTest(_InfoTest):
    category_key = "cat_system"
    name_key = "tn_kernel_version"
    _cmd = "uname -a"


# ── DTB 文件（主 DTB + Overlay） ─────────────────────────────────────────────
class DTBFileTest(TestCase):
    category_key = "cat_system"
    name_key = "tn_dtb_overlays"

    def _run(self):
        extlinux = "/boot/firmware/extlinux/extlinux.conf"
        rc, out, err = self.cmd(f"cat {extlinux} 2>/dev/null")
        if rc != 0:
            self.fail(t("sys_dtb_no_read", extlinux))
            return

        fdt = ""
        overlays = ""
        for line in out.splitlines():
            stripped = line.strip()
            if stripped.startswith("#"):
                continue
            parts = stripped.split()
            if not parts:
                continue
            if parts[0] == "fdt" and len(parts) > 1:
                fdt = parts[1]
            elif parts[0] == "fdtoverlays":
                overlays = " ".join(parts[1:])

        result = f"fdt: {fdt or '(none)'}"
        if overlays:
            result += f"  overlays: {overlays}"
        self.info(result)


# ── OS 版本 ─────────────────────────────────────────────────────────────────
class OSVersionTest(_InfoTest):
    category_key = "cat_system"
    name_key = "tn_os_version"
    _cmd = (
        "grep ^PRETTY_NAME /etc/os-release 2>/dev/null"
        " | cut -d= -f2 | tr -d '\"'"
    )


# ── 主机名 ──────────────────────────────────────────────────────────────────
class HostnameTest(_InfoTest):
    category_key = "cat_system"
    name_key = "tn_hostname"
    _cmd = "hostname"


# ── 系统运行时间 ─────────────────────────────────────────────────────────────
class UptimeTest(TestCase):
    category_key = "cat_system"
    name_key = "tn_uptime"

    def _run(self):
        rc, out, _ = self.cmd("cat /proc/uptime 2>/dev/null")
        if rc != 0 or not out.strip():
            self.info("uptime unknown")
            return
        try:
            total_secs = int(float(out.split()[0]))
        except (ValueError, IndexError):
            self.info(out.strip())
            return
        days, rem = divmod(total_secs, 86400)
        hours, rem = divmod(rem, 3600)
        minutes, seconds = divmod(rem, 60)
        if days > 0:
            self.info(t("sys_uptime_days", days, hours))
        elif hours > 0:
            self.info(t("sys_uptime_hours", hours, minutes))
        elif minutes > 0:
            self.info(t("sys_uptime_min", minutes))
        else:
            self.info(t("sys_uptime_sec", seconds))


# ── 根文件系统 ───────────────────────────────────────────────────────────────
class FilesystemTest(_InfoTest):
    category_key = "cat_system"
    name_key = "tn_root_filesystem"
    _cmd = "df -h / | tail -1"

    def _fmt(self, out):
        parts = out.split()
        if len(parts) >= 5:
            return t("sys_fs_fmt", parts[1], parts[2], parts[4])
        return out


# ── 内存 ────────────────────────────────────────────────────────────────────
class MemoryTest(_InfoTest):
    category_key = "cat_system"
    name_key = "tn_memory"
    _cmd = "free -m | awk '/^Mem/{print $2, $3, $4}'"

    def _fmt(self, out):
        parts = out.split()
        if len(parts) >= 3:
            return t("sys_mem_fmt", parts[0], parts[1], parts[2])
        return out


# ── CPU 核心数 ───────────────────────────────────────────────────────────────
class CPUCoreTest(_InfoTest):
    category_key = "cat_system"
    name_key = "tn_cpu_cores"
    _cmd = "nproc"

    def _fmt(self, out):
        return t("sys_cores_fmt", out)


# ── CPU 最大频率 ─────────────────────────────────────────────────────────────
class CPUFreqTest(TestCase):
    category_key = "cat_system"
    name_key = "tn_cpu_frequency"

    def _run(self):
        # Try cpufreq sysfs first (non-RT kernel with cpufreq driver)
        cpufreq = "/sys/devices/system/cpu/cpu0/cpufreq"
        rc_max, max_out, _ = self.cmd(f"cat {cpufreq}/cpuinfo_max_freq 2>/dev/null")
        rc_min, min_out, _ = self.cmd(f"cat {cpufreq}/cpuinfo_min_freq 2>/dev/null")
        rc_cur, cur_out, _ = self.cmd(f"cat {cpufreq}/scaling_cur_freq 2>/dev/null")
        if rc_max == 0 and max_out.strip():
            try:
                max_mhz = int(max_out.strip()) // 1000
                min_mhz = int(min_out.strip()) // 1000 if rc_min == 0 and min_out.strip() else None
                cur_mhz = int(cur_out.strip()) // 1000 if rc_cur == 0 and cur_out.strip() else None
                parts = [t("sys_freq_max", max_mhz)]
                if min_mhz is not None:
                    parts.append(t("sys_freq_min", min_mhz))
                if cur_mhz is not None:
                    parts.append(t("sys_freq_cur", cur_mhz))
                self.info("  ".join(parts))
                return
            except ValueError:
                pass
        # Fallback: use k3conf to query A53 core clock via TISCI (works on RT kernel)
        rc, out, _ = self.cmd("k3conf dump clock 135 0 2>/dev/null")
        if rc == 0 and out.strip():
            for line in out.splitlines():
                if "A53" in line and "CLK_STATE_READY" in line:
                    parts = line.split("|")
                    if len(parts) >= 6:
                        try:
                            hz = int(parts[5].strip())
                            cur_mhz = hz // 1_000_000
                            self.info(t("sys_freq_k3", cur_mhz))
                            return
                        except ValueError:
                            pass
        self.fail(t("sys_cannot_read_freq"))


# ── CPU 温度 ────────────────────────────────────────────────────────────────
class TempSensorTest(TestCase):
    category_key = "cat_system"
    name_key = "tn_cpu_temperature"

    def _run(self):
        rc, out, _ = self.cmd(
            "cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null"
        )
        if rc != 0 or not out.strip():
            self.fail(t("sys_cannot_read_temp"))
            return
        try:
            self.info(f"{int(out.strip()) / 1000.0:.1f} °C")
        except ValueError:
            self.fail(t("sys_parse_temp_fail", out.strip()))


# ── 以太网 MAC 地址 ──────────────────────────────────────────────────────────
class MACAddressTest(TestCase):
    category_key = "cat_system"
    name_key = "tn_ethernet_mac"

    def _run(self):
        # Find Ethernet interfaces: exclude lo and wlan*/wl* from /sys/class/net
        rc, out, _ = self.cmd(
            "for f in /sys/class/net/*/address; do"
            " iface=$(basename $(dirname $f));"
            " [ \"$iface\" = lo ] && continue;"
            " mac=$(cat $f 2>/dev/null);"
            " [ -n \"$mac\" ] && echo \"$iface $mac\";"
            " done"
        )
        if rc != 0 or not out.strip():
            self.fail(t("sys_no_net_iface"))
            return
        parts = []
        for line in out.strip().splitlines():
            cols = line.split()
            if len(cols) >= 2:
                parts.append(f"{cols[0]}: {cols[1]}")
        self.info("  ".join(parts) if parts else out.strip())


def get_tests(board, manual_confirm_fn=None):
    args = (board, manual_confirm_fn)
    return [
        FirmwareVersionTest(*args),
        KernelVersionTest(*args),
        DTBFileTest(*args),
        OSVersionTest(*args),
        HostnameTest(*args),
        UptimeTest(*args),
        FilesystemTest(*args),
        MemoryTest(*args),
        CPUCoreTest(*args),
        CPUFreqTest(*args),
        TempSensorTest(*args),
        MACAddressTest(*args),
    ]
