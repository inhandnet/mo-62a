"""
40Pin 扩展接口测试模块
"""

from tests.base import TestCase, TestResult


def _find_ehrpwm_chip(board) -> str:
    """动态查找 EHRPWM0 对应的 pwmchip 编号（非 pwmchip0）。

    返回 pwmchip 编号字符串，如 "1"；找不到则返回 "1"（默认 fallback）。
    """
    rc, out, err = board.run("ls /sys/class/pwm/")
    if rc != 0:
        return "1"
    chips = [line.strip() for line in out.strip().splitlines() if line.strip().startswith("pwmchip")]
    # pwmchip0 通常是风扇 PWM，找第一个不是 pwmchip0 的
    for chip in sorted(chips):
        if chip != "pwmchip0":
            return chip.replace("pwmchip", "")
    return "1"


class GpioChipTest(TestCase):
    category = "40Pin 扩展接口"
    name_key = "tn_gpio_chip"

    def _run(self):
        rc, out, err = self.cmd("ls /dev/gpiochip* 2>/dev/null")
        if rc != 0 or not out.strip():
            self.fail("未找到 /dev/gpiochip* 设备节点")
            return
        chips = out.strip().splitlines()
        if len(chips) < 3:
            self.fail(
                f"GPIO 芯片数量不足（期望 >= 3，实际 {len(chips)}）：{', '.join(c.strip() for c in chips)}"
            )
            return
        self.pass_(f"检测到 {len(chips)} 个 GPIO 芯片：{', '.join(c.strip() for c in chips)}")


class GpioOutputTest(TestCase):
    category = "40Pin 扩展接口"
    name_key = "tn_gpio_output"

    def _run(self):
        rc1, out1, err1 = self.cmd("gpioset gpiochip1 15=1")
        rc2, out2, err2 = self.cmd("gpioset gpiochip1 15=0")
        if rc1 != 0:
            self.fail(f"gpioset gpiochip1 15=1 失败（rc={rc1}）：{err1.strip()}")
            return
        if rc2 != 0:
            self.fail(f"gpioset gpiochip1 15=0 失败（rc={rc2}）：{err2.strip()}")
            return
        self.pass_("GPIO1_15（EXP Pin24）输出高/低电平正常")


class GpioLoopbackTest(TestCase):
    category = "40Pin 扩展接口"
    name_key = "tn_gpio_loopback"
    requires_manual = True

    def _run(self):
        confirmed = self.manual_confirm(
            "请短接 EXP Pin11（GPIO0_2）和 Pin12（GPIO0_3），然后点击确认"
        )
        if not confirmed:
            return

        # 输出高，读取
        self.cmd("gpioset gpiochip0 2=1")
        rc1, out1, err1 = self.cmd("gpioget gpiochip0 3")
        val_high = out1.strip()

        # 输出低，读取
        self.cmd("gpioset gpiochip0 2=0")
        rc2, out2, err2 = self.cmd("gpioget gpiochip0 3")
        val_low = out2.strip()

        if val_high != "1":
            self.fail(f"GPIO 回环失败：输出高时读到 '{val_high}'（期望 '1'）")
            return
        if val_low != "0":
            self.fail(f"GPIO 回环失败：输出低时读到 '{val_low}'（期望 '0'）")
            return
        self.pass_("GPIO 回环测试通过（Pin11→Pin12，高='1'，低='0'）")


class UartLoopbackTest(TestCase):
    category = "40Pin 扩展接口"
    name_key = "tn_uart_loopback"
    requires_manual = True

    def _run(self):
        confirmed = self.manual_confirm(
            "请短接 EXP Pin8（TX）和 Pin10（RX），然后点击确认"
        )
        if not confirmed:
            return

        py_cmd = (
            "python3 -c \""
            "import serial, time; "
            "s = serial.Serial('/dev/ttyS5', 115200, timeout=1); "
            "s.write(b'TEST'); "
            "time.sleep(0.1); "
            "data = s.read(10); "
            "s.close(); "
            "print(data)\""
        )
        rc, out, err = self.cmd(py_cmd, timeout=10)
        if rc != 0:
            self.fail(f"UART 回环测试命令失败（rc={rc}）：{err.strip()}")
            return
        if "TEST" not in out and b"TEST".decode() not in out:
            # 检查字节表示
            if "b'TEST'" not in out and "54455354" not in out.lower():
                self.fail(f"UART 回环数据不匹配，读回：{out.strip()}")
                return
        self.pass_(f"UART 回环测试通过，读回：{out.strip()}")


class PwmOutputTest(TestCase):
    category = "40Pin 扩展接口"
    name_key = "tn_pwm_output"

    def _run(self):
        chip = _find_ehrpwm_chip(self.board)
        chip_path = f"/sys/class/pwm/pwmchip{chip}"

        # Export PWM0
        self.cmd(f"echo 0 > {chip_path}/export 2>/dev/null")

        pwm_path = f"{chip_path}/pwm0"
        rc1, _, err1 = self.cmd(f"echo 1000000 > {pwm_path}/period")
        rc2, _, err2 = self.cmd(f"echo 500000 > {pwm_path}/duty_cycle")
        rc3, _, err3 = self.cmd(f"echo 1 > {pwm_path}/enable")

        if rc1 != 0:
            self.fail(f"设置 PWM period 失败（rc={rc1}）：{err1.strip()}")
            return
        if rc2 != 0:
            self.fail(f"设置 PWM duty_cycle 失败（rc={rc2}）：{err2.strip()}")
            return
        if rc3 != 0:
            self.fail(f"使能 PWM 失败（rc={rc3}）：{err3.strip()}")
            return

        # 禁用并清理
        self.cmd(f"echo 0 > {pwm_path}/enable 2>/dev/null")
        self.cmd(f"echo 0 > {chip_path}/unexport 2>/dev/null")

        self.pass_(f"EHRPWM0（pwmchip{chip}）输出 50% 占空比 1kHz 正常")


def get_tests(board, manual_confirm_fn=None):
    return [
        GpioChipTest(board, manual_confirm_fn),
        GpioOutputTest(board, manual_confirm_fn),
        GpioLoopbackTest(board, manual_confirm_fn),
        UartLoopbackTest(board, manual_confirm_fn),
        PwmOutputTest(board, manual_confirm_fn),
    ]
