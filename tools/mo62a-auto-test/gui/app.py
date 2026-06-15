"""主窗口 — QMainWindow + QStackedWidget 页面管理"""

from __future__ import annotations

from PySide6.QtWidgets import QMainWindow, QStackedWidget

from config.settings import APP_NAME, APP_VERSION, WIN_DEF_W, WIN_DEF_H, WIN_MIN_W, WIN_MIN_H
from config.i18n import set_lang


class MainWindow(QMainWindow):

    def __init__(self):
        super().__init__()
        self.setWindowTitle(f"{APP_NAME}  v{APP_VERSION}")
        self.resize(WIN_DEF_W, WIN_DEF_H)
        self.setMinimumSize(WIN_MIN_W, WIN_MIN_H)

        # 共享设备状态（连接成功后由 ConnectPage 写入）
        self.board       = None
        self.device_info = {}

        self._stack = QStackedWidget()
        self.setCentralWidget(self._stack)

        # 注册了 apply_lang() 的页面列表
        self._lang_pages: list = []

        self._init_pages()

    def _init_pages(self):
        from gui.welcome import WelcomePage
        from gui.connect import ConnectPage
        from gui.select  import SelectPage
        from gui.run     import RunPage

        self._welcome = WelcomePage(app=self)
        self._connect = ConnectPage(app=self)
        self._select  = SelectPage(app=self)
        self._run     = RunPage(app=self)

        self._welcome.continue_requested.connect(lambda: self.show_page("connect"))
        self._connect.back_requested.connect(lambda: self.show_page("welcome"))
        self._connect.connect_succeeded.connect(self._on_connect_succeeded)
        self._select.back_requested.connect(lambda: self.show_page("connect"))
        self._select.start_requested.connect(self._on_start_requested)
        self._run.back_requested.connect(lambda: self.show_page("select"))

        for page in (self._welcome, self._connect, self._select, self._run):
            self._stack.addWidget(page)
            self._lang_pages.append(page)

        self._stack.setCurrentWidget(self._welcome)

    # ── 页面切换 ──────────────────────────────────────────────────────────────
    def show_page(self, name: str):
        pages = {
            "welcome": self._welcome,
            "connect": self._connect,
            "select":  self._select,
            "run":     self._run,
        }
        page = pages.get(name)
        if page:
            self._stack.setCurrentWidget(page)
            if hasattr(page, "on_show"):
                page.on_show()

    # ── 全局语言切换 ─────────────────────────────────────────────────────────
    def apply_lang_all(self, lang: str):
        """任意页面切换语言时调用，同步所有已注册页面。"""
        set_lang(lang)
        for page in self._lang_pages:
            if hasattr(page, "apply_lang"):
                page.apply_lang()

    # ── 连接成功回调 ─────────────────────────────────────────────────────────
    def _on_connect_succeeded(self, board, device_info: dict):
        self.board       = board
        self.device_info = device_info
        # 新连接时重置摄像头分配缓存，确保重新检测
        from interface._camera import reset_cache
        reset_cache()
        self.show_page("select")

    def _on_start_requested(self, selected_keys: list):
        from interface.registry import get_selected_tests
        tests = get_selected_tests(self.board, selected_keys)
        self.show_page("run")
        self._run.start_tests(tests)
