"""
USB 测试模块
"""

from tests.base import TestCase, TestResult


class UsbHostTest(TestCase):
    category = "USB"
    name_key = "tn_usb_host"

    def _run(self):
        rc, out, err = self.cmd("lsusb 2>/dev/null")
        if rc != 0:
            self.fail(f"lsusb 命令失败（rc={rc}）")
            return
        device_list = out.strip() or "(无 USB 设备)"
        self.pass_(f"USB 设备列表：\n{device_list}")


class UsbStorageTest(TestCase):
    category = "USB"
    name_key = "tn_usb_storage"
    requires_manual = True

    def _run(self):
        # 提示用户插入 U 盘
        confirmed = self.manual_confirm("请插入 U 盘，然后点击确认")
        if not confirmed:
            return

        # 检查是否有 /dev/sd* 设备
        rc, out, err = self.cmd("ls /dev/sd* 2>/dev/null")
        if rc != 0 or not out.strip():
            self.fail("未检测到 USB 存储设备（/dev/sd*）")
            return

        sda_device = out.strip().splitlines()[0].strip()
        # 找到第一个分区
        rc2, out2, err2 = self.cmd("ls /dev/sda* 2>/dev/null | head -2")
        partitions = out2.strip().splitlines()
        # 优先使用分区（/dev/sda1），其次使用整块设备
        mount_dev = ""
        for p in partitions:
            if p.strip().endswith("1"):
                mount_dev = p.strip()
                break
        if not mount_dev:
            mount_dev = sda_device

        # 挂载并列出内容
        rc3, out3, err3 = self.cmd(
            f"mount {mount_dev} /mnt 2>/dev/null && ls /mnt && umount /mnt",
            timeout=15,
        )
        if rc3 != 0:
            self.fail(
                f"挂载 {mount_dev} 到 /mnt 失败（rc={rc3}）：{err3.strip()}"
            )
            return
        self.pass_(f"USB 存储设备 {mount_dev} 挂载读取成功")


class UsbKeyboardTest(TestCase):
    category = "USB"
    name_key = "tn_usb_keyboard"
    requires_manual = True

    def _run(self):
        confirmed = self.manual_confirm(
            "请确认 USB 键盘已连接并可识别（插入后点击）"
        )
        if not confirmed:
            return

        rc, out, err = self.cmd(
            "cat /proc/bus/input/devices | grep -i keyboard"
        )
        if not out.strip():
            self.fail("未在 /proc/bus/input/devices 中找到键盘设备")
            return
        self.pass_(f"检测到 USB 键盘设备：{out.strip().splitlines()[0]}")


def get_tests(board, manual_confirm_fn=None):
    return [
        UsbHostTest(board, manual_confirm_fn),
        UsbStorageTest(board, manual_confirm_fn),
        UsbKeyboardTest(board, manual_confirm_fn),
    ]
