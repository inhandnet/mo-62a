"""系统基础信息测试（5 项）

固件版本 / 内核版本 / CPU 核数 / CPU 温度 / 运行时间
均为 INFO 或 PASS/FAIL 判定，不需要人工确认。
"""

from __future__ import annotations
from config.i18n import t
from interface.base import TestCase


# ── 1. 固件版本 ───────────────────────────────────────────────────────────────
class FirmwareVersionTest(TestCase):
    category_key = "cat_system"
    name_key     = "tn_firmware_version"

    def _run(self):
        rc, out, _ = self.cmd("mo-version 2>/dev/null | head -2 | tr '\\n' '  '")
        if rc != 0 or not out.strip():
            self.fail(t("msg_mo_version_unavailable"))
            return
        self.info(out.strip())


# ── 2. 内核版本 ───────────────────────────────────────────────────────────────
class KernelVersionTest(TestCase):
    category_key = "cat_system"
    name_key     = "tn_kernel_version"

    def _run(self):
        rc, out, _ = self.cmd("uname -r")
        if rc != 0 or not out.strip():
            self.fail(t("msg_kernel_read_fail"))
            return
        self.info(out.strip())


# ── 3. CPU 核数 ───────────────────────────────────────────────────────────────
class CpuCoresTest(TestCase):
    category_key = "cat_system"
    name_key     = "tn_cpu_cores"

    def _run(self):
        rc, out, _ = self.cmd("nproc")
        if rc != 0 or not out.strip().isdigit():
            self.fail(t("msg_cpu_cores_fail"))
            return
        self.info(t("msg_cpu_cores", out.strip()))


# ── 4. CPU 温度 ───────────────────────────────────────────────────────────────
class CpuTempTest(TestCase):
    category_key = "cat_system"
    name_key     = "tn_cpu_temp"

    def _run(self):
        # 优先读 AM62Ax main0_thermal hwmon，回退到 thermal_zone0
        script = (
            "hwmon=$(grep -rl 'main0_thermal' /sys/class/hwmon/*/name 2>/dev/null"
            " | head -1 | xargs dirname 2>/dev/null);"
            " if [ -n \"$hwmon\" ]; then cat \"$hwmon/temp1_input\";"
            " else cat /sys/class/thermal/thermal_zone0/temp 2>/dev/null; fi"
        )
        rc, out, _ = self.cmd(script)
        if rc != 0 or not out.strip().lstrip("-").isdigit():
            self.fail(t("msg_cpu_temp_fail"))
            return
        self.info(f"{int(out.strip()) / 1000:.1f} °C")


# ── 5. 运行时间 ───────────────────────────────────────────────────────────────
class UptimeTest(TestCase):
    category_key = "cat_system"
    name_key     = "tn_uptime"

    def _run(self):
        # uptime -p 输出如 "up 2 hours, 15 minutes"
        # 回退：从 /proc/uptime 手动格式化
        rc, out, _ = self.cmd(
            "uptime -p 2>/dev/null || "
            "awk '{s=int($1); h=int(s/3600); m=int((s%3600)/60);"
            " printf \"%d h %d min\\n\", h, m}' /proc/uptime"
        )
        if rc != 0 or not out.strip():
            self.fail(t("msg_uptime_fail"))
            return
        # 去掉 "up " 前缀
        text = out.strip()
        if text.lower().startswith("up "):
            text = text[3:]
        self.info(text)


# ── 导出 ──────────────────────────────────────────────────────────────────────
def get_tests(board) -> list[TestCase]:
    """返回系统类别下所有测试项的实例列表。"""
    return [
        FirmwareVersionTest(board),
        KernelVersionTest(board),
        CpuCoresTest(board),
        CpuTempTest(board),
        UptimeTest(board),
    ]
