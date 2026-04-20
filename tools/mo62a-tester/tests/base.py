import time
from typing import Optional, Callable


class TestResult:
    PASS = "PASS"
    FAIL = "FAIL"
    SKIP = "SKIP"
    INFO = "INFO"
    MANUAL_PASS = "MANUAL_PASS"
    MANUAL_FAIL = "MANUAL_FAIL"


class TestCase:
    """所有测试模块的基类"""
    category = "misc"
    name_key = ""          # i18n key；子类设置此项
    requires_manual = False

    @property
    def name(self) -> str:
        from gui.i18n import t
        return t(self.name_key) if self.name_key else self.__class__.__name__

    def __init__(self, board, manual_confirm_fn: Optional[Callable] = None):
        # board: Board 或 SerialBoard 实例（都有 .run(cmd) 方法）
        # manual_confirm_fn: fn(prompt: str) -> bool，由 GUI 提供弹窗
        self.board = board
        self._manual_confirm = manual_confirm_fn
        self.message = ""
        self.status = TestResult.SKIP
        self.duration = 0.0

    def run(self) -> "TestCase":
        t0 = time.time()
        try:
            self._run()
        except Exception as e:
            self.status = TestResult.FAIL
            self.message = f"Exception: {e}"
        self.duration = time.time() - t0
        return self

    def _run(self):
        raise NotImplementedError

    def cmd(self, command: str, timeout: int = 30):
        """执行命令，返回 (rc, stdout, stderr)"""
        return self.board.run(command, timeout=timeout)

    def assert_rc(self, command: str, expected_rc: int = 0, msg: str = "") -> bool:
        rc, out, err = self.cmd(command)
        if rc != expected_rc:
            self.status = TestResult.FAIL
            self.message = msg or f"'{command}' returned {rc}, expected {expected_rc}\n{err}"
            return False
        return True

    def assert_contains(self, command: str, keyword: str, msg: str = "") -> bool:
        rc, out, err = self.cmd(command)
        if keyword not in out:
            self.status = TestResult.FAIL
            self.message = msg or f"'{keyword}' not found in output of '{command}'"
            return False
        return True

    def manual_confirm(self, prompt: str) -> bool:
        """请求人工确认，返回 True=通过 False=失败"""
        if self._manual_confirm:
            result = self._manual_confirm(prompt)
            self.status = TestResult.MANUAL_PASS if result else TestResult.MANUAL_FAIL
            return result
        # 无 GUI 时默认跳过
        self.status = TestResult.SKIP
        return False

    def pass_(self, message: str = ""):
        self.status = TestResult.PASS
        self.message = message

    def info(self, message: str):
        self.status = TestResult.INFO
        self.message = message

    def fail(self, message: str):
        self.status = TestResult.FAIL
        self.message = message

    def skip(self, reason: str = ""):
        self.status = TestResult.SKIP
        self.message = reason
