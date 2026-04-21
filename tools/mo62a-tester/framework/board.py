"""
SSH 连接封装，用于与 MO-62A 板子通信。
"""

import paramiko


class BoardConnectionError(Exception):
    """SSH 连接失败时抛出"""


class BoardCommandError(Exception):
    """命令执行出错时抛出"""


class Board:
    """SSH 连接封装，提供远程命令执行接口。"""

    def __init__(self):
        self._client: paramiko.SSHClient | None = None
        self.host: str = ""
        self.user: str = ""
        self.port: int = 22
        self._password: str = ""

    def connect(self, host: str, user: str, password: str, port: int = 22) -> None:
        """建立 SSH 连接。

        Args:
            host: 目标板子 IP 或主机名
            user: SSH 用户名
            password: SSH 密码
            port: SSH 端口，默认 22

        Raises:
            BoardConnectionError: 连接失败
        """
        self.host = host
        self.user = user
        self.port = port
        self._password = password

        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())

        try:
            client.connect(
                hostname=host,
                port=port,
                username=user,
                password=password,
                timeout=15,
                banner_timeout=20,
                auth_timeout=15,
                look_for_keys=False,
                allow_agent=False,
            )
        except paramiko.AuthenticationException as e:
            raise BoardConnectionError(f"Authentication failed for {user}@{host}: {e}") from e
        except paramiko.SSHException as e:
            raise BoardConnectionError(f"SSH error connecting to {host}:{port}: {e}") from e
        except OSError as e:
            raise BoardConnectionError(f"Network error connecting to {host}:{port}: {e}") from e

        self._client = client

    def run(self, cmd: str, timeout: int = 30) -> tuple[int, str, str]:
        """在板子上执行命令。

        Args:
            cmd: Shell 命令字符串
            timeout: 超时秒数，默认 30

        Returns:
            (returncode, stdout, stderr) 元组

        Raises:
            BoardConnectionError: 未连接
            BoardCommandError: 命令执行异常
        """
        if self._client is None:
            raise BoardConnectionError("Not connected. Call connect() first.")

        try:
            stdin, stdout, stderr = self._client.exec_command(cmd, timeout=timeout)
            stdin.close()

            rc = stdout.channel.recv_exit_status()
            out = stdout.read().decode("utf-8", errors="replace")
            err = stderr.read().decode("utf-8", errors="replace")
            return rc, out, err

        except paramiko.SSHException as e:
            raise BoardCommandError(f"Command execution error: {e}") from e

    def close(self) -> None:
        """关闭 SSH 连接。"""
        if self._client is not None:
            try:
                self._client.close()
            finally:
                self._client = None

    def change_password(self, new_password: str) -> None:
        """修改当前连接用户的密码（分配 PTY，交互式执行 passwd）。

        Raises:
            BoardCommandError: 修改失败
        """
        if self._client is None:
            raise BoardConnectionError("Not connected.")

        import time
        channel = self._client.get_transport().open_session()
        channel.get_pty()
        channel.exec_command("passwd")

        def _recv_until(keyword: str, timeout: float = 10.0) -> str:
            buf = ""
            deadline = time.monotonic() + timeout
            while time.monotonic() < deadline:
                if channel.recv_ready():
                    buf += channel.recv(4096).decode("utf-8", errors="replace")
                    if keyword.lower() in buf.lower():
                        return buf
                time.sleep(0.05)
            raise BoardCommandError(f"Timeout waiting for '{keyword}' in passwd output")

        try:
            _recv_until("password:")
            channel.send(self._password + "\n")

            _recv_until("new password:")
            channel.send(new_password + "\n")

            _recv_until("new password:")
            channel.send(new_password + "\n")

            out = _recv_until("successfully", timeout=8)
            if "successfully" not in out.lower():
                raise BoardCommandError(f"passwd did not confirm success: {out}")
        finally:
            channel.close()

    def get_file(self, remote_path: str) -> bytes:
        """通过 SFTP 从板子下载文件内容。

        Raises:
            BoardConnectionError: 未连接
            BoardCommandError: 下载失败
        """
        if self._client is None:
            raise BoardConnectionError("Not connected. Call connect() first.")
        try:
            sftp = self._client.open_sftp()
            try:
                with sftp.open(remote_path, "rb") as f:
                    return f.read()
            finally:
                sftp.close()
        except (paramiko.SSHException, OSError) as e:
            raise BoardCommandError(f"SFTP download failed: {e}") from e

    def put_file(self, local_path: str, remote_path: str) -> None:
        """通过 SFTP 上传文件到板子。

        Args:
            local_path: 本机文件路径
            remote_path: 远端目标路径

        Raises:
            BoardConnectionError: 未连接
            BoardCommandError: 上传失败
        """
        if self._client is None:
            raise BoardConnectionError("Not connected. Call connect() first.")

        try:
            sftp = self._client.open_sftp()
            try:
                sftp.put(local_path, remote_path)
            finally:
                sftp.close()
        except (paramiko.SSHException, OSError) as e:
            raise BoardCommandError(f"SFTP upload failed: {e}") from e

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False
