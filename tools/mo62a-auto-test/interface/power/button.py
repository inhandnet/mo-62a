"""S1 电源按键测试 — 自动检测人工短按

流程：
  1. 弹出一个带 10 秒倒计时进度条的无按钮提示对话框
  2. 后台把 watcher 脚本写入 /tmp/ 并启动，持续 EVIOCGRAB 独占监听 event0
  3. watcher 检测到 KEY_POWER 后写标志文件
  4. 主进程轮询检查标志文件，检测到或超时后关闭对话框并返回结果
"""

from __future__ import annotations
import base64
import time
from config.i18n import t
from interface.base import TestCase


_WATCHER_SCRIPT = r"""import struct, os, select, fcntl, sys
evdev = sys.argv[1]
flag  = sys.argv[2]
EVIOCGRAB = 0x40044590
fd = os.open(evdev, os.O_RDONLY | os.O_NONBLOCK)
fcntl.ioctl(fd, EVIOCGRAB, 1)
fmt = 'llHHi'
sz = struct.calcsize(fmt)
found = False
while not found:
    r, _, _ = select.select([fd], [], [], 0.2)
    if fd in r:
        data = os.read(fd, sz)
        if len(data) == sz:
            _, _, typ, code, val = struct.unpack(fmt, data)
            if typ == 1 and code == 116:
                found = True
fcntl.ioctl(fd, EVIOCGRAB, 0)
os.close(fd)
with open(flag, 'w') as f:
    f.write('1')
"""


class PowerButtonTest(TestCase):
    category_key = "cat_power"
    name_key     = "tn_button"

    TIMEOUT_S = 10.0
    POLL_S    = 0.2

    def _run(self):
        # 找到 tps6594-pwrbutton 对应的事件设备
        rc, evdev, _ = self.cmd(
            "for i in /sys/class/input/input*; do "
            "  name=$(cat $i/name 2>/dev/null); "
            "  [ \"$name\" = \"tps6594-pwrbutton\" ] && "
            "    echo /dev/input/$(basename $i/event*) && break; "
            "done"
        )
        evdev = evdev.strip()
        if not evdev:
            self.fail(t("msg_button_evdev_missing"))
            return

        watcher_path = "/tmp/mo62a_key_watcher.py"
        flag = "/tmp/.mo62a_s1_pressed"

        # 清理旧标志
        self.cmd(f"rm -f {flag}")

        # 把 watcher 脚本以 base64 写入设备临时文件
        b64 = base64.b64encode(_WATCHER_SCRIPT.encode("utf-8")).decode("ascii")
        rc, _, err = self.cmd(
            f"printf '%s' '{b64}' | base64 -d > {watcher_path}"
        )
        if rc != 0:
            self.fail(t("msg_button_write_fail", err.strip()[:80]))
            return

        # 启动 watcher 后台进程
        rc, _, err = self.cmd(f"python3 {watcher_path} {evdev} {flag} &")
        if rc != 0:
            self.fail(t("msg_button_start_fail", err.strip()[:80]))
            return

        # 弹出提示对话框（无按钮，后台自动关闭）
        close_dlg = self.manual_prompt("manual_button_prompt")
        update_dlg = self.manual_prompt_progress

        pressed = False
        t0 = time.monotonic()
        try:
            while time.monotonic() - t0 < self.TIMEOUT_S:
                elapsed = time.monotonic() - t0
                remaining = max(0, int(self.TIMEOUT_S - elapsed))
                pct = int(min(100, (elapsed / self.TIMEOUT_S) * 100))
                if update_dlg:
                    update_dlg(pct, remaining, "")

                rc, _, _ = self.cmd(f"test -f {flag}")
                if rc == 0:
                    pressed = True
                    break
                time.sleep(self.POLL_S)
        except Exception:
            pass
        finally:
            if close_dlg:
                close_dlg()
            # 清理 watcher 和标志
            self.cmd(
                f"pkill -f 'python3 {watcher_path}' 2>/dev/null; "
                f"rm -f {flag} {watcher_path}"
            )

        elapsed = time.monotonic() - t0
        if pressed:
            self.pass_(t("msg_button_detected", f"{elapsed:.1f}"))
        else:
            self.fail(t("msg_button_timeout", self.TIMEOUT_S))


def get_tests(board) -> list:
    return [PowerButtonTest(board)]
