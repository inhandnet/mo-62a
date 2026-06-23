"""
interface/base.py — 所有测试项的公共基类

使用方式：
    每个具体测试继承 TestCase，设置 category_key / name_key，
    重写 _run() 方法，在其中调用 self.cmd() 执行命令，
    用 self.pass_() / self.fail() / self.skip() / self.info() 报告结果。

示例：
    class FirmwareVersionTest(TestCase):
        category_key = "cat_system"
        name_key     = "tn_firmware_version"

        def _run(self):
            rc, out, _ = self.cmd("mo-version 2>/dev/null | head -1")
            if rc == 0 and out.strip():
                self.pass_(out.strip())
            else:
                self.fail("mo-version 命令失败或无输出")
"""

from __future__ import annotations

import time
from dataclasses import dataclass, field
from pathlib import Path


# ── 测试结果状态 ──────────────────────────────────────────────────────────────
class Status:
    PASS = "PASS"   # 自动检测通过
    FAIL = "FAIL"   # 自动检测失败
    SKIP = "SKIP"   # 跳过（条件不满足，不计入失败）
    INFO = "INFO"   # 仅展示信息，不判定通过/失败


# ── 单次测试结果 ──────────────────────────────────────────────────────────────
@dataclass
class Result:
    status:   str   = Status.SKIP
    message:  str   = ""
    duration: float = 0.0          # 秒
    images:   list  = field(default_factory=list)   # 测试产生的图片路径列表


# ── 测试基类 ──────────────────────────────────────────────────────────────────
class TestCase:
    """所有测试项的基类，子类只需重写 _run()。"""

    category_key: str = ""   # i18n key，对应测试分类名称
    name_key:     str = ""   # i18n key，对应测试项名称

    def __init__(self, board):
        """
        Args:
            board: SSHBoard 实例，提供 run(cmd) 方法执行远端命令。
        """
        self.board    = board
        self._status  = Status.SKIP
        self._message = ""
        self._images: list[Path] = []   # 测试产生的图片，Reporter 会嵌入 HTML
        self._manual_confirm_fn: callable | None = None
        self._manual_prompt_fn: callable | None = None
        self._manual_prompt_progress_fn: callable | None = None

    def set_manual_confirm(self, fn: callable) -> None:
        """由运行框架注入人工确认回调函数。"""
        self._manual_confirm_fn = fn

    def set_manual_prompt(self, fn: callable) -> None:
        """由运行框架注入人工提示对话框回调函数（返回关闭函数）。"""
        self._manual_prompt_fn = fn

    def set_manual_prompt_progress(self, fn: callable) -> None:
        """由运行框架注入人工提示对话框进度更新回调函数。"""
        self._manual_prompt_progress_fn = fn

    # ── 对外接口 ──────────────────────────────────────────────────────────────
    @property
    def name(self) -> str:
        """返回当前语言的测试项名称。"""
        from config.i18n import t
        return t(self.name_key) if self.name_key else self.__class__.__name__

    @property
    def category(self) -> str:
        """返回当前语言的分类名称。"""
        from config.i18n import t
        return t(self.category_key) if self.category_key else ""

    def run(self) -> Result:
        """
        执行测试，自动计时并捕获未预期异常。
        返回 Result(status, message, duration, images)。
        """
        t0 = time.monotonic()
        try:
            self._run()
        except Exception as e:
            self._status  = Status.FAIL
            self._message = f"未预期异常: {e}"
        duration = time.monotonic() - t0
        return Result(self._status, self._message, duration, list(self._images))

    def attach_image(self, path) -> None:
        """子类调用以附加测试产物图片到报告中。"""
        self._images.append(Path(path))

    def manual_confirm(self, prompt: str) -> bool:
        """弹出对话框请求人工确认。返回 True=通过，False=失败。"""
        if self._manual_confirm_fn:
            return self._manual_confirm_fn(prompt)
        # 无回调时默认失败，避免自动化流程误判
        self._status = Status.FAIL
        self._message = "未配置人工确认回调"
        return False

    def manual_prompt(self, prompt: str, show_progress: bool = True) -> callable | None:
        """弹出只提示、无按钮的对话框，返回关闭函数；无回调返回 None。"""
        if self._manual_prompt_fn:
            return self._manual_prompt_fn(prompt, show_progress)
        return None

    def manual_prompt_progress(self, percent: int, remaining_s: int, status_text: str = "") -> None:
        """更新提示对话框的进度条和状态文本。"""
        if self._manual_prompt_progress_fn:
            self._manual_prompt_progress_fn(percent, remaining_s, status_text)

    # ── 子类实现区 ────────────────────────────────────────────────────────────
    def _run(self) -> None:
        """子类重写此方法，实现具体测试逻辑。"""
        raise NotImplementedError

    # ── 命令执行 ──────────────────────────────────────────────────────────────
    def cmd(self, command: str, timeout: int = 30) -> tuple[int, str, str]:
        """
        在设备上执行 shell 命令。

        Returns:
            (returncode, stdout, stderr)
        """
        return self.board.run(command, timeout=timeout)

    def local_cmd(self, command: str, timeout: int = 60) -> tuple[int, str, str]:
        """在测试主机（本地）执行命令，不经过 SSH。"""
        import subprocess
        try:
            r = subprocess.run(
                command, shell=True, capture_output=True,
                text=True, timeout=timeout,
            )
            return r.returncode, r.stdout, r.stderr
        except subprocess.TimeoutExpired:
            return -1, "", "local_cmd timeout"

    # ── 常用断言 ──────────────────────────────────────────────────────────────
    def assert_rc(self, command: str, expected: int = 0,
                  msg: str = "") -> bool:
        """执行命令，若返回码不符合预期则自动 fail。返回是否通过。"""
        rc, out, err = self.cmd(command)
        if rc != expected:
            self.fail(msg or f"`{command}` 返回 {rc}，期望 {expected}\n{err.strip()}")
            return False
        return True

    def assert_contains(self, command: str, keyword: str,
                        msg: str = "") -> bool:
        """执行命令，若输出中不含关键字则自动 fail。返回是否通过。"""
        rc, out, err = self.cmd(command)
        if keyword not in out:
            self.fail(msg or f"`{command}` 输出中未找到 '{keyword}'")
            return False
        return True

    # ── 结果设置 ──────────────────────────────────────────────────────────────
    def pass_(self, message: str = "") -> None:
        """标记测试通过。"""
        self._status  = Status.PASS
        self._message = message

    def fail(self, message: str = "") -> None:
        """标记测试失败。"""
        self._status  = Status.FAIL
        self._message = message

    def skip(self, reason: str = "") -> None:
        """标记测试跳过（条件不满足，不计入失败）。"""
        self._status  = Status.SKIP
        self._message = reason

    def info(self, message: str = "") -> None:
        """仅记录信息，不影响通过/失败判定。"""
        self._status  = Status.INFO
        self._message = message
