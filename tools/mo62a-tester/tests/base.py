import time
from typing import Optional, Callable


class SliderRequest:
    """滑动条手工测试请求对象，传给 _manual_input_fn 触发滑动条对话框。"""
    def __init__(self, min_val: int, max_val: int, on_change: Callable[[int], None],
                 unit: str = "%", initial_val: int = None):
        self.min_val = min_val
        self.max_val = max_val
        self.on_change = on_change  # callable(int) -> None，在 daemon 线程中调用
        self.unit = unit
        self.initial_val = initial_val if initial_val is not None else min_val


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

    def __init__(self, board, manual_confirm_fn: Optional[Callable] = None,
                 manual_input_fn: Optional[Callable] = None):
        self.board = board
        self._manual_confirm = manual_confirm_fn
        self._manual_input_fn = manual_input_fn
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
        self.status = TestResult.SKIP
        return False

    def manual_slider_confirm(self, prompt: str, min_val: int, max_val: int,
                              on_change: Callable[[int], None],
                              initial_val: int = None) -> bool:
        """弹出带滑动条的对话框，on_change(value) 在滑动停稳后被调用（daemon 线程）。"""
        req = SliderRequest(min_val, max_val, on_change, initial_val=initial_val)
        if self._manual_input_fn:
            result = self._manual_input_fn(prompt, None, req)
            return result == "pass"
        return False

    def manual_image_confirm(self, prompt: str, image_bytes: bytes) -> bool:
        """弹出带截图的对话框，让用户对比截图与实际显示是否一致。
        image_bytes 通过 _manual_input_fn(prompt, None, image_bytes) 传递，
        返回 True=一致，False=不一致。"""
        if self._manual_input_fn:
            result = self._manual_input_fn(prompt, None, image_bytes)
            return result == "pass"
        return False

    def manual_input(self, prompt: str, choices: list = None,
                     password: bool = False) -> Optional[str]:
        """请求用户输入文本，choices 非空时显示下拉选择，password=True 时隐藏输入。
        返回用户输入的字符串，取消时返回 None。"""
        if self._manual_input_fn:
            return self._manual_input_fn(prompt, choices, password)
        return None

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
