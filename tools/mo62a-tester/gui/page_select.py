"""第2页：测试项选择（三态复选框）"""
import importlib
import tkinter as tk
from tkinter import ttk, messagebox

from gui.i18n import t
from gui.fonts import F_NORMAL, F_BOLD, F_SMALL, F_TITLE, F_HEADER, F_CODE

COLOR_PRIMARY = "#2c5f9e"
COLOR_BG      = "#f5f7fa"
COLOR_CAT_BG  = "#e8ecf4"

# (大类 i18n key, 模块路径)
TEST_CATEGORIES = [
    ("cat_system",   "tests.test_system"),
    ("cat_network",  "tests.test_network"),
    ("cat_leds",     "tests.test_leds"),
    ("cat_hdmi",     "tests.test_hdmi"),
    ("cat_services", "tests.test_services"),
    ("cat_rtc",      "tests.test_rtc"),
    ("cat_fan",      "tests.test_fan"),
    ("cat_i2c",      "tests.test_i2c"),
    ("cat_storage",  "tests.test_storage"),
]

_CHECK_OFF  = "[ ]"
_CHECK_ON   = "[x]"
_CHECK_HALF = "[-]"


class _CategoryRow:
    def __init__(self, key: str, module_path: str):
        self.key = key              # i18n key
        self.module_path = module_path
        self.state_var = tk.IntVar(value=1)  # 0=off,1=on,2=half
        self.btn: tk.Button | None = None
        self.name_lbl: tk.Label | None = None
        self.children: list["_ItemRow"] = []

    @property
    def name(self) -> str:
        return t(self.key)

    def update_state_from_children(self) -> None:
        checked = sum(1 for c in self.children if c.var.get())
        total = len(self.children)
        if checked == 0:
            self.state_var.set(0)
        elif checked == total:
            self.state_var.set(1)
        else:
            self.state_var.set(2)
        self._refresh_btn()

    def _refresh_btn(self) -> None:
        if self.btn is None:
            return
        self.btn.config(text={0: _CHECK_OFF, 1: _CHECK_ON, 2: _CHECK_HALF}.get(
            self.state_var.get(), _CHECK_OFF
        ))

    def toggle(self) -> None:
        if self.state_var.get() in (0, 2):
            new_val, new_state = True, 1
        else:
            new_val, new_state = False, 0
        self.state_var.set(new_state)
        for child in self.children:
            child.var.set(new_val)
        self._refresh_btn()

    def update_label(self) -> None:
        """语言切换后刷新大类名称标签。"""
        if self.name_lbl is not None:
            count = len(self.children)
            self.name_lbl.config(
                text=f"{t(self.key)}  ({t('cat_items', count)})"
            )


class _ItemRow:
    def __init__(self, tc, category: _CategoryRow):
        self.tc = tc
        self.category = category
        self.var = tk.BooleanVar(value=True)
        self.chk: tk.Checkbutton | None = None
        self.name_lbl: tk.Label | None = None

    @property
    def name(self) -> str:
        return getattr(self.tc, "name", str(self.tc))

    @property
    def requires_manual(self) -> bool:
        return bool(getattr(self.tc, "requires_manual", False))


class SelectPage(tk.Frame):

    def __init__(self, parent: tk.Widget, app) -> None:
        super().__init__(parent, bg=COLOR_BG)
        self.app = app
        self._categories: list[_CategoryRow] = []
        self._build_ui()
        app.register_lang_callback(self.update_language)

    # ------------------------------------------------------------------
    def _build_ui(self) -> None:
        self.rowconfigure(1, weight=1)
        self.columnconfigure(0, weight=1)

        # ── 标题栏 ──────────────────────────────────────────────────────
        header = tk.Frame(self, bg=COLOR_PRIMARY, height=50)
        header.grid(row=0, column=0, sticky="ew")
        header.grid_propagate(False)
        header.columnconfigure(0, weight=1)

        self._header_lbl = tk.Label(
            header, text=t("select_header"),
            bg=COLOR_PRIMARY, fg="white", font=F_HEADER,
        )
        self._header_lbl.grid(row=0, column=0, sticky="w", padx=20, pady=12)

        btn_frame = tk.Frame(header, bg=COLOR_PRIMARY)
        btn_frame.grid(row=0, column=1, padx=12, pady=8, sticky="e")

        self._select_all_btn = tk.Button(
            btn_frame, text=t("select_all"),
            font=F_SMALL, padx=10, pady=3, relief="groove", cursor="hand2",
            command=self._select_all,
        )
        self._select_all_btn.pack(side="left", padx=(0, 6))

        self._select_none_btn = tk.Button(
            btn_frame, text=t("select_none"),
            font=F_SMALL, padx=10, pady=3, relief="groove", cursor="hand2",
            command=self._select_none,
        )
        self._select_none_btn.pack(side="left")

        # ── 滚动区域 ────────────────────────────────────────────────────
        scroll_container = tk.Frame(self, bg=COLOR_BG)
        scroll_container.grid(row=1, column=0, sticky="nsew", padx=10, pady=8)
        scroll_container.rowconfigure(0, weight=1)
        scroll_container.columnconfigure(0, weight=1)

        canvas = tk.Canvas(scroll_container, bg=COLOR_BG, highlightthickness=0)
        vsb = ttk.Scrollbar(scroll_container, orient="vertical", command=canvas.yview)
        canvas.configure(yscrollcommand=vsb.set)

        canvas.grid(row=0, column=0, sticky="nsew")
        vsb.grid(row=0, column=1, sticky="ns")

        self._inner = tk.Frame(canvas, bg=COLOR_BG)
        self._inner_id = canvas.create_window((0, 0), window=self._inner, anchor="nw")

        self._inner.bind("<Configure>", lambda e: canvas.configure(
            scrollregion=canvas.bbox("all")
        ))
        canvas.bind("<Configure>", lambda e: canvas.itemconfig(
            self._inner_id, width=e.width
        ))

        def _scroll(event):
            if event.num == 4:
                canvas.yview_scroll(-1, "units")
            elif event.num == 5:
                canvas.yview_scroll(1, "units")
            else:
                canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")

        canvas.bind("<MouseWheel>", _scroll)
        canvas.bind("<Button-4>", _scroll)
        canvas.bind("<Button-5>", _scroll)
        self._canvas = canvas

        # ── 底部工具栏 ──────────────────────────────────────────────────
        footer = tk.Frame(self, bg=COLOR_BG)
        footer.grid(row=2, column=0, sticky="ew", padx=10, pady=(0, 10))
        footer.columnconfigure(1, weight=1)

        self._count_var = tk.StringVar(value=t("selected_count", 0, 0))
        tk.Label(
            footer, textvariable=self._count_var,
            bg=COLOR_BG, font=F_NORMAL,
        ).grid(row=0, column=0, padx=8, pady=6, sticky="w")

        self._back_btn = tk.Button(
            footer, text=t("back"),
            font=F_NORMAL, padx=12, pady=5, relief="groove", cursor="hand2",
            command=lambda: self.app.show_page("connect"),
        )
        self._back_btn.grid(row=0, column=1, sticky="e", padx=(0, 6))

        self._start_btn = tk.Button(
            footer, text=t("start_test"),
            bg=COLOR_PRIMARY, fg="white", font=F_BOLD,
            padx=14, pady=5, relief="flat", cursor="hand2",
            command=self._on_start,
        )
        self._start_btn.grid(row=0, column=2, sticky="e", padx=(0, 8), pady=6)

    # ------------------------------------------------------------------
    # 语言切换
    # ------------------------------------------------------------------
    def update_language(self) -> None:
        self._header_lbl.config(text=t("select_header"))
        self._select_all_btn.config(text=t("select_all"))
        self._select_none_btn.config(text=t("select_none"))
        self._back_btn.config(text=t("back"))
        self._start_btn.config(text=t("start_test"))
        # 刷新大类名称 + 各测试项名称
        for cat in self._categories:
            cat.update_label()
            for item in cat.children:
                if item.name_lbl is not None:
                    item.name_lbl.config(text=item.name)
        # 刷新计数
        self._update_count()

    # ------------------------------------------------------------------
    # 数据加载
    # ------------------------------------------------------------------
    def on_show(self) -> None:
        self._load_tests()

    def _load_tests(self) -> None:
        for widget in self._inner.winfo_children():
            widget.destroy()
        self._categories.clear()

        board = self.app.board

        def _dummy_confirm(prompt: str) -> bool:
            return True

        for cat_key, mod_path in TEST_CATEGORIES:
            cat = _CategoryRow(cat_key, mod_path)
            self._categories.append(cat)

            try:
                mod = importlib.import_module(mod_path)
                test_instances = mod.get_tests(board, _dummy_confirm)
            except Exception:
                test_instances = []

            for tc in test_instances:
                item = _ItemRow(tc, cat)
                cat.children.append(item)

            self._render_category(cat)

        self._update_count()

    def _render_category(self, cat: _CategoryRow) -> None:
        inner = self._inner

        cat_row = tk.Frame(inner, bg=COLOR_CAT_BG)
        cat_row.pack(fill="x", pady=(4, 0))

        btn = tk.Button(
            cat_row, text=_CHECK_ON, font=F_NORMAL,
            bg=COLOR_CAT_BG, relief="flat", bd=0, cursor="hand2",
            command=lambda c=cat: self._toggle_category(c),
        )
        btn.pack(side="left", padx=(8, 2), pady=4)
        cat.btn = btn

        count = len(cat.children)
        name_lbl = tk.Label(
            cat_row,
            text=f"{t(cat.key)}  ({t('cat_items', count)})",
            bg=COLOR_CAT_BG, font=F_BOLD,
        )
        name_lbl.pack(side="left", pady=4)
        cat.name_lbl = name_lbl

        for item in cat.children:
            self._render_item(item, inner)

        cat.update_state_from_children()

    def _render_item(self, item: _ItemRow, parent: tk.Widget) -> None:
        item_row = tk.Frame(parent, bg=COLOR_BG)
        item_row.pack(fill="x", pady=1)

        tk.Frame(item_row, bg=COLOR_BG, width=24).pack(side="left")

        chk = tk.Checkbutton(
            item_row, variable=item.var,
            bg=COLOR_BG, activebackground=COLOR_BG,
            command=lambda c=item.category: self._on_item_toggle(c),
        )
        chk.pack(side="left")
        item.chk = chk

        item.name_lbl = tk.Label(
            item_row, text=item.name,
            bg=COLOR_BG, font=F_NORMAL, anchor="w",
        )
        item.name_lbl.pack(side="left")

        if item.requires_manual:
            tk.Label(
                item_row, text=t("manual_badge"),
                bg=COLOR_BG, fg="#3498db", font=F_SMALL,
            ).pack(side="left")

    # ------------------------------------------------------------------
    # 事件处理
    # ------------------------------------------------------------------
    def _toggle_category(self, cat: _CategoryRow) -> None:
        cat.toggle()
        self._update_count()

    def _on_item_toggle(self, cat: _CategoryRow) -> None:
        cat.update_state_from_children()
        self._update_count()

    def _select_all(self) -> None:
        for cat in self._categories:
            cat.state_var.set(1)
            if cat.btn:
                cat.btn.config(text=_CHECK_ON)
            for item in cat.children:
                item.var.set(True)
        self._update_count()

    def _select_none(self) -> None:
        for cat in self._categories:
            cat.state_var.set(0)
            if cat.btn:
                cat.btn.config(text=_CHECK_OFF)
            for item in cat.children:
                item.var.set(False)
        self._update_count()

    def _update_count(self) -> None:
        total = sum(len(c.children) for c in self._categories)
        selected = sum(
            1 for c in self._categories for item in c.children if item.var.get()
        )
        self._count_var.set(t("selected_count", selected, total))

    def _on_start(self) -> None:
        selected = [
            item for cat in self._categories
            for item in cat.children if item.var.get()
        ]
        if not selected:
            messagebox.showwarning(
                t("no_test_warn_title"), t("no_test_warn_msg"), parent=self
            )
            return
        self.app.selected_tests = selected
        self.app.show_page("run")

    # ------------------------------------------------------------------
    def get_selected_tests(self, board, manual_confirm_fn, manual_input_fn=None):
        results = []
        for cat in self._categories:
            try:
                mod = importlib.import_module(cat.module_path)
                all_tests = mod.get_tests(board, manual_confirm_fn,
                                          manual_input_fn=manual_input_fn)
            except TypeError:
                all_tests = mod.get_tests(board, manual_confirm_fn)
            except Exception:
                all_tests = []

            name_map = {getattr(tc, "name", str(tc)): tc for tc in all_tests}
            for item in cat.children:
                if item.var.get() and item.name in name_map:
                    results.append(name_map[item.name])
        return results
