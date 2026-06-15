"""风扇测试 — PWM 控制 + Tach 转速

流程：
  1. 停止 fancontrol 服务
  2. 切换到手动模式（pwm1_enable=1）
  3. 三档循环：100% / 50% / 0%，每档等 2.5s 后读 RPM
  4. 恢复自动模式，重启 fancontrol
判定：
  100% > 50%，且 100% RPM > 1500（确认风扇能转）
"""

from __future__ import annotations
import time
from interface.base import TestCase


class FanControlTest(TestCase):
    category_key = "cat_power"
    name_key     = "tn_fan_control"

    SETTLE_S       = 2.5    # 每档 PWM 后等待稳定时间
    MIN_FULL_RPM   = 1500   # 100% 时的最低期望 RPM

    def _run(self):
        # ── 查找 pwmfan hwmon 路径 ────────────────────────────────────────────
        rc, hwmon, _ = self.cmd(
            "for d in /sys/class/hwmon/hwmon*; do "
            "  [ \"$(cat $d/name 2>/dev/null)\" = pwmfan ] && echo $d && break; "
            "done"
        )
        hwmon = hwmon.strip()
        if not hwmon:
            self.fail("未找到 pwmfan hwmon 设备")
            return

        pwm_path     = f"{hwmon}/pwm1"
        pwm_en_path  = f"{hwmon}/pwm1_enable"
        rpm_path     = f"{hwmon}/fan1_input"

        # 保存原始模式以便恢复
        _, orig_mode, _ = self.cmd(f"cat {pwm_en_path} 2>/dev/null")
        orig_mode = orig_mode.strip() or "2"

        # 停止 fancontrol 服务
        self.cmd("systemctl stop fancontrol 2>/dev/null", timeout=10)

        try:
            # 进入手动模式
            self.cmd(f"echo 1 > {pwm_en_path}")

            # 测五档：0% / 25% / 50% / 75% / 100%
            steps = [("0%", 0), ("25%", 64), ("50%", 128), ("75%", 192), ("100%", 255)]
            results = []   # [(label, pwm_val, rpm)]
            for label, pwm_val in steps:
                self.cmd(f"echo {pwm_val} > {pwm_path}")
                time.sleep(self.SETTLE_S)

                rc, rpm_out, _ = self.cmd(f"cat {rpm_path} 2>/dev/null")
                try:
                    rpm = int(rpm_out.strip())
                except ValueError:
                    rpm = -1
                results.append((label, pwm_val, rpm))

        finally:
            # 恢复原始模式
            self.cmd(f"echo {orig_mode} > {pwm_en_path}", timeout=5)
            self.cmd("systemctl start fancontrol 2>/dev/null", timeout=10)

        # ── 判定 ──────────────────────────────────────────────────────────────
        pwms = "/".join(lbl for lbl, _, _ in results)
        rpms = "/".join(str(rpm) for _, _, rpm in results)
        msg  = f"{pwms} → {rpms} RPM"

        if any(r < 0 for _, _, r in results):
            self.fail(f"无法读取 RPM: {msg}")
            return

        rpm_0   = results[0][2]
        rpm_100 = results[-1][2]

        if rpm_100 < self.MIN_FULL_RPM:
            self.fail(f"{msg}（100% RPM 低于 {self.MIN_FULL_RPM}）")
            return
        if rpm_100 <= rpm_0:
            self.fail(f"{msg}（100% 转速未高于 0%）")
            return

        self.pass_(msg)


def get_tests(board) -> list:
    return [FanControlTest(board)]
