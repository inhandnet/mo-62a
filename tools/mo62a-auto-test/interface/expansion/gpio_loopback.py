"""40-pin GPIO 回环测试

使用 microSD-default（全 GPIO）启动标签，通过 libgpiod Python 绑定对 12 对引脚做电平翻转验证。
排除 pin27/pin28（摄像头 I2C2）。

每对测试：
  1. A 输出 0/1，B 输入读回
  2. B 输出 0/1，A 输入读回
"""

from __future__ import annotations
import base64
from interface.base import TestCase
from config.i18n import t


# 12 对回环：格式 (output_pin, input_pin)
# pin 映射为 (chip, line)，来自工厂实测治具
_LOOPBACK_PAIRS: list[tuple[tuple[str, int], tuple[str, int]]] = [
    (("gpiochip0", 20), ("gpiochip0", 19)),  # Pin3  -> Pin5
    (("gpiochip1", 39), ("gpiochip2", 23)),  # Pin7  -> Pin11
    (("gpiochip1", 42), ("gpiochip2", 22)),  # Pin13 -> Pin15
    (("gpiochip2", 18), ("gpiochip2", 19)),  # Pin19 -> Pin21
    (("gpiochip1", 36), ("gpiochip1", 33)),  # Pin29 -> Pin31
    (("gpiochip2", 13), ("gpiochip1", 91)),  # Pin33 -> Pin35
    (("gpiochip1", 41), ("gpiochip2",  2)),  # Pin37 -> Pin40
    (("gpiochip2", 25), ("gpiochip2", 24)),  # Pin8  -> Pin10
    (("gpiochip2",  0), ("gpiochip1", 38)),  # Pin12 -> Pin14
    (("gpiochip1", 40), ("gpiochip1", 14)),  # Pin18 -> Pin22
    (("gpiochip2", 15), ("gpiochip2", 16)),  # Pin24 -> Pin26
    (("gpiochip2",  9), ("gpiochip2",  5)),  # Pin36 -> Pin38
]


_TEST_SCRIPT = r"""
import gpiod
from gpiod.line import Direction, Value

PAIRS = [
    (("gpiochip0", 20), ("gpiochip0", 19)),
    (("gpiochip1", 39), ("gpiochip2", 23)),
    (("gpiochip1", 42), ("gpiochip2", 22)),
    (("gpiochip2", 18), ("gpiochip2", 19)),
    (("gpiochip1", 36), ("gpiochip1", 33)),
    (("gpiochip2", 13), ("gpiochip1", 91)),
    (("gpiochip1", 41), ("gpiochip2",  2)),
    (("gpiochip2", 25), ("gpiochip2", 24)),
    (("gpiochip2",  0), ("gpiochip1", 38)),
    (("gpiochip1", 40), ("gpiochip1", 14)),
    (("gpiochip2", 15), ("gpiochip2", 16)),
    (("gpiochip2",  9), ("gpiochip2",  5)),
]

chip_lines = {}
for out, inp in PAIRS:
    for chip, line in (out, inp):
        chip_lines.setdefault(chip, set()).add(line)

chip_requests = {}
for chip, lines in chip_lines.items():
    config = {line: gpiod.LineSettings(direction=Direction.INPUT) for line in lines}
    chip_requests[chip] = gpiod.request_lines(
        f"/dev/{chip}",
        consumer="mo62a_gpio_loopback",
        config=config
    )

errors = []
for idx, (out, inp) in enumerate(PAIRS, 1):
    out_chip, out_line = out
    in_chip, in_line = inp

    for val in (0, 1):
        try:
            chip_requests[out_chip].reconfigure_lines({
                out_line: gpiod.LineSettings(
                    direction=Direction.OUTPUT,
                    output_value=Value.ACTIVE if val else Value.INACTIVE
                )
            })
            chip_requests[in_chip].reconfigure_lines({
                in_line: gpiod.LineSettings(direction=Direction.INPUT)
            })
            values = chip_requests[in_chip].get_values([in_line])
            read = 1 if values[0] == Value.ACTIVE else 0
            if read != val:
                errors.append(idx)
                break
        except Exception:
            errors.append(idx)
            break

    if idx in errors:
        continue

    for val in (0, 1):
        try:
            chip_requests[in_chip].reconfigure_lines({
                in_line: gpiod.LineSettings(
                    direction=Direction.OUTPUT,
                    output_value=Value.ACTIVE if val else Value.INACTIVE
                )
            })
            chip_requests[out_chip].reconfigure_lines({
                out_line: gpiod.LineSettings(direction=Direction.INPUT)
            })
            values = chip_requests[out_chip].get_values([out_line])
            read = 1 if values[0] == Value.ACTIVE else 0
            if read != val:
                errors.append(idx)
                break
        except Exception:
            errors.append(idx)
            break

for req in chip_requests.values():
    req.release()

if errors:
    print("FAIL:" + ",".join(f"pair{i}" for i in sorted(set(errors))))
else:
    print(f"PASS:{len(PAIRS)}")
"""


class GpioLoopbackTest(TestCase):
    category_key = "cat_expansion"
    name_key     = "tn_gpio_loopback"

    def _run(self):
        script_path = "/tmp/mo62a_gpio_loopback.py"
        b64 = base64.b64encode(_TEST_SCRIPT.encode("utf-8")).decode("ascii")

        rc, _, err = self.cmd(
            f"printf '%s' '{b64}' | base64 -d > {script_path}"
        )
        if rc != 0:
            self.fail(f"写入 GPIO 测试脚本失败: {err.strip()[:80]}")
            return

        rc, out, err = self.cmd(f"python3 {script_path}", timeout=30)
        self.cmd(f"rm -f {script_path}")

        if rc != 0:
            self.fail(f"GPIO 测试执行失败: {err.strip()[:80]}")
            return

        line = out.strip()
        if line.startswith("PASS:"):
            self.pass_(t("gpio_pass_summary", line[5:]))
        elif line.startswith("FAIL:"):
            pairs = line[5:].split(",")
            self.fail(t("gpio_fail_summary", ", ".join(pairs)))
        else:
            self.fail(t("gpio_unknown_output", line[:120]))


def get_tests(board) -> list:
    return [GpioLoopbackTest(board)]
