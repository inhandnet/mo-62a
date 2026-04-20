"""MO-62A 测试工具主窗口"""
import tkinter as tk
from tkinter import ttk

from gui.i18n import t, set_lang, get_lang
from gui.fonts import apply_global, F_SMALL, F_BOLD

COLOR_PRIMARY = "#2c5f9e"


class App(tk.Tk):
    """主窗口，管理页面切换与语言切换。"""

    def __init__(self):
        super().__init__()

        # 全局等宽字体（必须在任何 widget 创建前调用）
        apply_global(self)

        # 共享状态
        self.board = None          # Board 或 SerialBoard 实例
        self.device_info = {}      # 设备信息 dict
        self.selected_tests = []   # 选中的 _ItemRow 列表
        self.reporter = None       # Reporter 实例

        # 语言变更回调列表（各页面注册）
        self._lang_callbacks: list = []

        self.title(t("app_title"))
        self.resizable(True, True)

        width, height = 900, 560
        self.geometry(f"{width}x{height}")
        self._center_window(width, height)
        self.minsize(800, 500)

        # ── 页面容器 ─────────────────────────────────────────────────────
        self._container = tk.Frame(self)
        self._container.pack(fill="both", expand=True)

        self._container.grid_rowconfigure(0, weight=1)
        self._container.grid_columnconfigure(0, weight=1)

        from gui.page_connect import ConnectPage
        from gui.page_select import SelectPage
        from gui.page_run import RunPage

        self._pages: dict[str, tk.Frame] = {}
        for name, PageClass in [
            ("connect", ConnectPage),
            ("select",  SelectPage),
            ("run",     RunPage),
        ]:
            page = PageClass(self._container, self)
            self._pages[name] = page
            page.grid(row=0, column=0, sticky="nsew")

        self.show_page("connect")

    # ------------------------------------------------------------------
    def _center_window(self, width: int, height: int) -> None:
        self.update_idletasks()
        sw = self.winfo_screenwidth()
        sh = self.winfo_screenheight()
        x = (sw - width) // 2
        y = (sh - height) // 2
        self.geometry(f"{width}x{height}+{x}+{y}")

    def show_page(self, name: str) -> None:
        page = self._pages[name]
        page.tkraise()
        if hasattr(page, "on_show"):
            page.on_show()

    # ------------------------------------------------------------------
    # 语言切换
    # ------------------------------------------------------------------
    def register_lang_callback(self, fn) -> None:
        """各页面在 __init__ 中调用，注册语言变更回调。"""
        self._lang_callbacks.append(fn)

    def _toggle_lang(self) -> None:
        new_lang = "zh" if get_lang() == "en" else "en"
        set_lang(new_lang)
        self.title(t("app_title"))
        for cb in self._lang_callbacks:
            try:
                cb()
            except Exception:
                pass
