"""
串口连接封装，用于通过串口与 MO-62A 板子通信。
"""

import time
from typing import Optional

import serial
import serial.tools.list_ports

from config import SERIAL_BAUD

# Shell 提示符结束标志
_PROMPTS = (b"# ", b"$ ")
_PROMPT_TIMEOUT_STEP = 0.05  # 每次读取等待时间（秒）


def list_ports() -> list[tuple[str, str]]:
    """列出系统中所有可用串口。

    Returns:
        [(port_name, description), ...] 列表
    """
    ports = serial.tools.list_ports.comports()
    return [(p.device, p.description or "") for p in sorted(ports, key=lambda x: x.device)]


class SerialBoard:
    """串口连接封装，提供与 Board（SSH）兼容的 run() 接口。"""

    def __init__(self):
        self._serial: Optional[serial.Serial] = None

    def connect(self, port: str, baud: int = SERIAL_BAUD) -> None:
        """打开串口连接。

        Args:
            port: 串口设备路径，如 /dev/ttyUSB0 或 COM3
            baud: 波特率，默认 115200

        Raises:
            serial.SerialException: 串口打开失败
        """
        self._serial = serial.Serial(
            port=port,
            baudrate=baud,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=_PROMPT_TIMEOUT_STEP,
        )
        # 等待板子就绪，发送一个回车触发提示符
        time.sleep(0.3)
        self._serial.write(b"\n")
        self._serial.flush()
        self._read_until_prompt(timeout=5.0)

    def run(self, cmd: str, timeout: int = 30) -> tuple[int, str, str]:
        """通过串口执行命令，等待 shell 提示符后返回。

        Args:
            cmd: Shell 命令字符串（不含换行）
            timeout: 等待命令完成的超时秒数

        Returns:
            (0, stdout, "") 元组，串口模式下 returncode 固定为 0

        Raises:
            RuntimeError: 未连接或超时
        """
        if self._serial is None:
            raise RuntimeError("Not connected. Call connect() first.")

        # 清空接收缓冲区
        self._serial.reset_input_buffer()

        # 发送命令
        line = (cmd.strip() + "\n").encode("utf-8")
        self._serial.write(line)
        self._serial.flush()

        # 读取响应直到出现提示符
        output = self._read_until_prompt(timeout=float(timeout))

        # 去掉回显的命令本身（第一行）
        lines = output.splitlines(keepends=True)
        if lines and cmd.strip() in lines[0]:
            lines = lines[1:]
        # 去掉最后的提示符行
        if lines:
            last = lines[-1].rstrip()
            if last.endswith("#") or last.endswith("$"):
                lines = lines[:-1]

        stdout = "".join(lines)
        return 0, stdout, ""

    def close(self) -> None:
        """关闭串口连接。"""
        if self._serial is not None:
            try:
                self._serial.close()
            finally:
                self._serial = None

    # ------------------------------------------------------------------
    # 内部方法
    # ------------------------------------------------------------------

    def _read_until_prompt(self, timeout: float = 30.0) -> str:
        """持续读取串口数据，直到出现 shell 提示符或超时。

        Returns:
            接收到的全部文本（UTF-8，替换无效字符）
        """
        buf = bytearray()
        deadline = time.monotonic() + timeout

        while time.monotonic() < deadline:
            chunk = self._serial.read(256)
            if chunk:
                buf.extend(chunk)
                # 检查缓冲区末尾是否出现提示符
                tail = bytes(buf[-8:])
                for prompt in _PROMPTS:
                    if prompt in tail:
                        return buf.decode("utf-8", errors="replace")
            # 没有数据时短暂等待
            time.sleep(_PROMPT_TIMEOUT_STEP)

        raise RuntimeError(f"Timed out waiting for shell prompt after {timeout}s")

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False
