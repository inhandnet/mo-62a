"""SSH 连接封装 — 连接后自动切换到 root 持久 shell

连接流程：
  1. paramiko SSH 登录（普通用户）
  2. invoke_shell() 打开持久 PTY
  3. sudo -s 切换到 root（使用连接密码）
  4. 禁用 echo，清空提示符

所有 run() 调用均在 root shell 中执行，无需每次 sudo。
"""

from __future__ import annotations

import re
import socket
import time

_MARKER = "__XCMD_END__"
# ANSI 转义 + 括号粘贴模式序列 [?2004h/l
_ANSI   = re.compile(
    r'\x1b(?:'
    r'\[\?[0-9]+[hl]'           # [?2004h [?2004l 等
    r'|\[[0-9;]*[mKHABCDEFGJnrsuhl]'
    r'|\([AB012]|\)[AB012]'
    r'|[78=><NO]'
    r')'
)


class SSHError(Exception):
    pass


class SSHBoard:
    """持久 root shell SSH 连接封装。"""

    def __init__(self):
        self._client = None
        self._shell  = None
        self.host     = ""
        self.user     = ""
        self.port     = 22
        self._password = ""

    # ── 连接 ──────────────────────────────────────────────────────────────────
    def connect(self, host: str, user: str, password: str, port: int = 22) -> None:
        import paramiko
        self.host      = host
        self.user      = user
        self.port      = port
        self._password = password

        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        try:
            client.connect(
                hostname=host, port=port, username=user, password=password,
                timeout=15, banner_timeout=20, auth_timeout=15,
                look_for_keys=False, allow_agent=False,
            )
        except paramiko.AuthenticationException as e:
            raise SSHError(f"认证失败: {e}") from e
        except paramiko.SSHException as e:
            raise SSHError(f"SSH 错误: {e}") from e
        except OSError as e:
            raise SSHError(f"网络错误: {e}") from e

        self._client = client
        self._open_root_shell(password)

    def _open_root_shell(self, password: str) -> None:
        """打开持久 PTY 并切换到 root。"""
        shell = self._client.invoke_shell(term="vt100", width=512, height=50)
        shell.settimeout(2)
        self._shell = shell

        # 等待初始 banner / 提示符
        time.sleep(0.8)
        self._drain()

        # 切换到 root：sudo -i 会提示输入密码，随即发送
        shell.send("sudo -i\n")
        time.sleep(0.5)
        self._drain(0.5)
        shell.send(f"{password}\n")
        time.sleep(1.0)
        self._drain()

        # 关闭 echo，清空提示符，关闭括号粘贴模式
        shell.send(
            "stty -echo; "
            "export PS1=''; export PS2=''; "
            "bind 'set enable-bracketed-paste off' 2>/dev/null; "
            "printf '\\e[?2004l'\n"
        )
        time.sleep(0.3)
        self._drain()

    # ── 命令执行 ──────────────────────────────────────────────────────────────
    def run(self, cmd: str, timeout: int = 30) -> tuple[int, str, str]:
        """
        在 root shell 中执行命令。

        Returns:
            (returncode, stdout, stderr)
            注：PTY 模式下 stderr 与 stdout 合并，stderr 返回空字符串。
        """
        if not self._shell:
            raise SSHError("未连接，请先调用 connect()")

        marker = _MARKER
        self._shell.settimeout(0.5)
        self._shell.send(f"{cmd}\necho '{marker}':$?\n")

        buf      = ""
        deadline = time.monotonic() + timeout

        while time.monotonic() < deadline:
            try:
                data = self._shell.recv(65536)
                if data:
                    buf += data.decode("utf-8", errors="replace")
                    if f"{marker}:" in buf:
                        break
            except socket.timeout:
                pass

        # 解析结果
        idx = buf.find(f"{marker}:")
        if idx < 0:
            return (-1, self._clean(buf), "timeout")

        stdout_raw = buf[:idx]
        rc_part    = buf[idx + len(marker) + 1:].split("\n")[0].strip()
        try:
            rc = int(rc_part)
        except (ValueError, AttributeError):
            rc = 0

        return (rc, self._clean(stdout_raw), "")

    # ── 工具 ──────────────────────────────────────────────────────────────────
    def _drain(self, wait: float = 0.8) -> None:
        """清空缓冲区中的待读数据。"""
        self._shell.settimeout(0.1)
        deadline = time.monotonic() + wait
        while time.monotonic() < deadline:
            try:
                self._shell.recv(4096)
            except socket.timeout:
                break

    @staticmethod
    def _clean(text: str) -> str:
        """去除 ANSI 转义码和多余的回车符。"""
        text = _ANSI.sub("", text)
        text = text.replace("\r", "")
        lines = [l.rstrip() for l in text.split("\n")]
        while lines and not lines[0].strip():
            lines.pop(0)
        while lines and not lines[-1].strip():
            lines.pop()
        return "\n".join(lines)

    # ── 文件传输 ──────────────────────────────────────────────────────────────
    def get_file(self, remote_path: str) -> bytes:
        """通过 SFTP 从设备下载文件，返回字节内容。"""
        import io
        sftp = self._client.open_sftp()
        try:
            buf = io.BytesIO()
            sftp.getfo(remote_path, buf)
            return buf.getvalue()
        finally:
            sftp.close()

    def put_file(self, local_path: str, remote_path: str) -> None:
        """通过 SFTP 上传文件到设备（使用 /tmp 中转，再由 root shell 移动到目标位置）。"""
        import os
        tmp = f"/tmp/_upload_{os.path.basename(local_path)}"
        sftp = self._client.open_sftp()
        try:
            sftp.put(local_path, tmp)
        finally:
            sftp.close()
        remote_dir = remote_path.rsplit("/", 1)[0] if "/" in remote_path else "."
        self.run(f"mkdir -p {remote_dir} && mv {tmp} {remote_path}")

    # ── 关闭 ──────────────────────────────────────────────────────────────────
    def close(self) -> None:
        if self._shell:
            try:
                self._shell.close()
            except Exception:
                pass
            finally:
                self._shell = None
        if self._client:
            try:
                self._client.close()
            except Exception:
                pass
            finally:
                self._client = None

    @property
    def connected(self) -> bool:
        return self._shell is not None and self._client is not None

    def __enter__(self): return self
    def __exit__(self, *_): self.close(); return False
