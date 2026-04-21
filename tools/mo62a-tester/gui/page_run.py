"""第3页：运行进度 + 日志 + 报告保存"""
import os
import threading
import time
import webbrowser
import tkinter as tk
from tkinter import ttk, filedialog, messagebox
from datetime import datetime

from config import REPORT_DIR
from gui.i18n import t
from gui.fonts import F_NORMAL, F_BOLD, F_SMALL, F_TITLE, F_HEADER, F_CODE

COLOR_PRIMARY  = "#2c5f9e"
COLOR_BG       = "#f5f7fa"
COLOR_PASS     = "#27ae60"
COLOR_FAIL     = "#e74c3c"
COLOR_SKIP     = "#95a5a6"
COLOR_INFO     = "#8e44ad"
COLOR_MANUAL_P = "#3498db"
COLOR_MANUAL_F = "#e67e22"


_STATUS_COLORS = {
    "PASS":        COLOR_PASS,
    "FAIL":        COLOR_FAIL,
    "SKIP":        COLOR_SKIP,
    "INFO":        COLOR_INFO,
    "MANUAL_PASS": COLOR_MANUAL_P,
    "MANUAL_FAIL": COLOR_MANUAL_F,
    "ERROR":       COLOR_FAIL,
}

_STATUS_KEYS = {
    "PASS":        "status_pass",
    "FAIL":        "status_fail",
    "SKIP":        "status_skip",
    "INFO":        "status_info",
    "MANUAL_PASS": "status_manual_pass",
    "MANUAL_FAIL": "status_manual_fail",
    "ERROR":       "status_error",
}


class RunPage(tk.Frame):

    def __init__(self, parent: tk.Widget, app) -> None:
        super().__init__(parent, bg=COLOR_BG)
        self.app = app
        self._results: list[dict] = []
        self._total = 0
        self._done = 0
        self._reporter = None

        self._manual_event = threading.Event()
        self._manual_result: bool = False
        self._manual_input_result: str | None = None

        self._build_ui()
        app.register_lang_callback(self.update_language)

    # ------------------------------------------------------------------
    def _build_ui(self) -> None:
        self.rowconfigure(2, weight=1)
        self.rowconfigure(3, weight=1)
        self.columnconfigure(0, weight=1)

        # ── 标题栏 ──────────────────────────────────────────────────────
        header = tk.Frame(self, bg=COLOR_PRIMARY, height=50)
        header.grid(row=0, column=0, sticky="ew")
        header.grid_propagate(False)
        self._header_lbl = tk.Label(
            header, text=t("run_header"),
            bg=COLOR_PRIMARY, fg="white", font=F_HEADER,
        )
        self._header_lbl.pack(side="left", padx=20, pady=12)


        # ── 进度条 ──────────────────────────────────────────────────────
        prog_frame = tk.Frame(self, bg=COLOR_BG)
        prog_frame.grid(row=1, column=0, sticky="ew", padx=16, pady=(10, 4))
        prog_frame.columnconfigure(0, weight=1)

        self._progress_var = tk.DoubleVar(value=0)
        ttk.Progressbar(
            prog_frame, variable=self._progress_var, maximum=100, length=600,
        ).grid(row=0, column=0, sticky="ew", pady=(0, 4))

        self._prog_label_var = tk.StringVar(value="0/0")
        tk.Label(
            prog_frame, textvariable=self._prog_label_var,
            bg=COLOR_BG, font=F_NORMAL,
        ).grid(row=1, column=0, sticky="w")

        # ── 结果列表 ────────────────────────────────────────────────────
        result_frame = tk.Frame(self, bg=COLOR_BG)
        result_frame.grid(row=2, column=0, sticky="nsew", padx=16, pady=(6, 4))
        result_frame.rowconfigure(1, weight=1)
        result_frame.columnconfigure(0, weight=1)

        self._result_list_lbl = tk.Label(
            result_frame, text=t("result_list"),
            bg=COLOR_BG, font=F_BOLD, anchor="w",
        )
        self._result_list_lbl.grid(row=0, column=0, sticky="w", pady=(0, 2))

        cols = ("status", "name", "duration", "message")
        self._tree = ttk.Treeview(
            result_frame, columns=cols, show="headings",
            height=8, selectmode="browse",
        )
        self._tree_col_keys = [
            ("status",   "col_status",   100, "center"),
            ("name",     "col_name",     200, "w"),
            ("duration", "col_duration",  70, "center"),
            ("message",  "col_message",  360, "w"),
        ]
        for col, key, width, anchor in self._tree_col_keys:
            self._tree.heading(col, text=t(key))
            self._tree.column(col, width=width, anchor=anchor)

        for tag, color in _STATUS_COLORS.items():
            self._tree.tag_configure(tag, foreground=color)

        vsb_r = ttk.Scrollbar(result_frame, orient="vertical", command=self._tree.yview)
        hsb_r = ttk.Scrollbar(result_frame, orient="horizontal", command=self._tree.xview)
        self._tree.configure(yscrollcommand=vsb_r.set, xscrollcommand=hsb_r.set)
        self._tree.grid(row=1, column=0, sticky="nsew")
        vsb_r.grid(row=1, column=1, sticky="ns")
        hsb_r.grid(row=2, column=0, sticky="ew")

        # ── 日志区 ──────────────────────────────────────────────────────
        log_frame = tk.Frame(self, bg=COLOR_BG)
        log_frame.grid(row=3, column=0, sticky="nsew", padx=16, pady=(4, 4))
        log_frame.rowconfigure(1, weight=1)
        log_frame.columnconfigure(0, weight=1)

        self._log_lbl = tk.Label(
            log_frame, text=t("log_output"),
            bg=COLOR_BG, font=F_BOLD, anchor="w",
        )
        self._log_lbl.grid(row=0, column=0, sticky="w", pady=(0, 2))

        self._log_text = tk.Text(
            log_frame, height=6, font=F_CODE,
            bg="#1e1e2e", fg="#cdd6f4",
            insertbackground="white", state="disabled", wrap="none",
        )
        vsb_l = ttk.Scrollbar(log_frame, orient="vertical", command=self._log_text.yview)
        hsb_l = ttk.Scrollbar(log_frame, orient="horizontal", command=self._log_text.xview)
        self._log_text.configure(yscrollcommand=vsb_l.set, xscrollcommand=hsb_l.set)
        self._log_text.grid(row=1, column=0, sticky="nsew")
        vsb_l.grid(row=1, column=1, sticky="ns")
        hsb_l.grid(row=2, column=0, sticky="ew")

        # ── 底部工具栏 ──────────────────────────────────────────────────
        footer = tk.Frame(self, bg=COLOR_BG)
        footer.grid(row=4, column=0, sticky="ew", padx=16, pady=(4, 10))
        footer.columnconfigure(1, weight=1)

        self._summary_var = tk.StringVar(value="")
        tk.Label(
            footer, textvariable=self._summary_var,
            bg=COLOR_BG, font=F_NORMAL, anchor="w",
        ).grid(row=0, column=0, sticky="w", padx=(0, 10))

        self._save_btn = tk.Button(
            footer, text=t("save_report"),
            font=F_SMALL, padx=10, pady=4, relief="groove", cursor="hand2",
            state="disabled", command=self._on_save_report,
        )
        self._save_btn.grid(row=0, column=1, sticky="e", padx=(0, 6))

        self._retry_btn = tk.Button(
            footer, text=t("retry"),
            font=F_SMALL, padx=10, pady=4, relief="groove", cursor="hand2",
            command=self._on_retry,
        )
        self._retry_btn.grid(row=0, column=2, sticky="e", padx=(0, 6))

        self._done_btn = tk.Button(
            footer, text=t("done"),
            font=F_BOLD, bg=COLOR_PRIMARY, fg="white",
            padx=14, pady=4, relief="flat", cursor="hand2",
            command=lambda: self.app.show_page("connect"),
        )
        self._done_btn.grid(row=0, column=3, sticky="e")

    # ------------------------------------------------------------------
    # 语言切换
    # ------------------------------------------------------------------
    def update_language(self) -> None:
        self._header_lbl.config(text=t("run_header"))
        self._result_list_lbl.config(text=t("result_list"))
        self._log_lbl.config(text=t("log_output"))
        for col, key, _, _ in self._tree_col_keys:
            self._tree.heading(col, text=t(key))
        self._save_btn.config(text=t("save_report"))
        self._retry_btn.config(text=t("retry"))
        self._done_btn.config(text=t("done"))

    # ------------------------------------------------------------------
    def on_show(self) -> None:
        self._reset_ui()
        self._start_tests()

    def _reset_ui(self) -> None:
        self._results.clear()
        self._done = 0
        self._total = 0
        self._tree.delete(*self._tree.get_children())
        self._log_text.config(state="normal")
        self._log_text.delete("1.0", "end")
        self._log_text.config(state="disabled")
        self._progress_var.set(0)
        self._prog_label_var.set("0/0")
        self._summary_var.set("")
        self._save_btn.config(state="disabled")
        self._reporter = None

    # ------------------------------------------------------------------
    def _start_tests(self) -> None:
        board = self.app.board

        from gui.page_select import SelectPage
        select_page = next(
            (p for p in self.app._pages.values() if isinstance(p, SelectPage)),
            None,
        )

        if select_page is not None:
            test_instances = select_page.get_selected_tests(
                board, self._manual_confirm_fn, self._manual_input_fn)
        else:
            test_instances = [item.tc for item in self.app.selected_tests]

        self._total = len(test_instances)
        self._prog_label_var.set(f"0/{self._total}")

        try:
            from framework.reporter import Reporter
            self._reporter = Reporter(self.app.device_info)
        except Exception:
            self._reporter = None

        threading.Thread(
            target=self._run_worker, args=(test_instances,), daemon=True
        ).start()

    def _run_worker(self, test_instances: list) -> None:
        _run_start = time.monotonic()
        for tc in test_instances:
            name = getattr(tc, "name", str(tc))
            self.after(0, lambda n=name: self._log(t("running", n)))

            start = time.monotonic()
            try:
                result = tc.run()
            except Exception as exc:
                class _Fake:
                    status = "ERROR"
                    message = str(exc)
                    duration = time.monotonic() - start
                result = _Fake()

            duration = getattr(result, "duration", time.monotonic() - start)
            status   = getattr(result, "status",   "ERROR")
            message  = getattr(result, "message",  "")

            if self._reporter is not None:
                try:
                    cat_key = getattr(tc, "category_key", None)
                    cat = t(cat_key) if cat_key else getattr(tc, "category", "")
                    self._reporter.add_result(cat, name, status, message, duration)
                except Exception:
                    pass

            result_dict = {"name": name, "status": status,
                           "message": message, "duration": duration}
            self._results.append(result_dict)
            self.after(0, lambda r=result_dict: self._on_result(r))
            if message:
                self.after(0, lambda m=message: self._log(m))

        total_s = time.monotonic() - _run_start
        mins, secs = divmod(int(total_s), 60)
        self.app.device_info["test_time"] = (
            t("dur_min_sec", mins, secs) if mins else t("dur_sec_only", secs)
        )
        self.after(0, self._on_all_done)

    # ------------------------------------------------------------------
    def _on_result(self, result: dict) -> None:
        status = result["status"]
        label = t(_STATUS_KEYS.get(status, status))
        dur_str = t("duration_s", f"{result['duration']:.1f}")

        self._tree.insert(
            "", "end",
            values=(label, result["name"], dur_str, result["message"]),
            tags=(status,),
        )
        children = self._tree.get_children()
        if children:
            self._tree.see(children[-1])

        self._done += 1
        pct = (self._done / self._total * 100) if self._total else 0
        self._progress_var.set(pct)
        self._prog_label_var.set(f"{self._done}/{self._total}")

    def _on_all_done(self) -> None:
        counts = {"PASS": 0, "FAIL": 0, "SKIP": 0, "ERROR": 0,
                  "MANUAL_PASS": 0, "MANUAL_FAIL": 0}
        for r in self._results:
            counts[r["status"]] = counts.get(r["status"], 0) + 1

        passed  = counts["PASS"] + counts["MANUAL_PASS"] + counts.get("INFO", 0)
        failed  = counts["FAIL"] + counts["MANUAL_FAIL"] + counts["ERROR"]
        skipped = counts["SKIP"]

        self._summary_var.set(t("summary", passed, failed, skipped))
        self._save_btn.config(state="normal")
        self._log(t("all_done", passed, failed, skipped))

    def _log(self, message: str) -> None:
        ts = datetime.now().strftime("%H:%M:%S")
        self._log_text.config(state="normal")
        self._log_text.insert("end", f"[{ts}] {message}\n")
        self._log_text.see("end")
        self._log_text.config(state="disabled")

    # ------------------------------------------------------------------
    def _manual_confirm_fn(self, prompt: str) -> bool:
        self._manual_event.clear()
        self._manual_result = False

        def _show():
            dlg = tk.Toplevel(self)
            dlg.title(t("manual_confirm_title"))
            dlg.resizable(False, False)
            dlg.grab_set()
            dlg.transient(self)

            dlg.update_idletasks()
            w, h = 420, 190
            sw, sh = dlg.winfo_screenwidth(), dlg.winfo_screenheight()
            dlg.geometry(f"{w}x{h}+{(sw-w)//2}+{(sh-h)//2}")

            tk.Label(
                dlg, text=prompt, font=F_NORMAL,
                wraplength=380, justify="left", pady=20, padx=20,
            ).pack(fill="both", expand=True)

            bf = tk.Frame(dlg)
            bf.pack(pady=(0, 16))

            def _pass():
                self._manual_result = True
                dlg.destroy()
                self._manual_event.set()

            def _fail():
                self._manual_result = False
                dlg.destroy()
                self._manual_event.set()

            tk.Button(
                bf, text=t("manual_pass"),
                bg=COLOR_PASS, fg="white", font=F_BOLD,
                padx=20, pady=6, relief="flat", cursor="hand2",
                command=_pass,
            ).pack(side="left", padx=10)

            tk.Button(
                bf, text=t("manual_fail"),
                bg=COLOR_FAIL, fg="white", font=F_BOLD,
                padx=20, pady=6, relief="flat", cursor="hand2",
                command=_fail,
            ).pack(side="left", padx=10)

            dlg.protocol("WM_DELETE_WINDOW", _fail)

        self.after(0, _show)
        self._manual_event.wait()
        return self._manual_result

    # ------------------------------------------------------------------
    def _manual_input_fn(self, prompt: str, choices: list = None,
                         password=False) -> "str | None":
        self._manual_event.clear()
        self._manual_input_result = None

        # ── 滑动条模式（password 为 SliderRequest 时触发）────────────
        if hasattr(password, "on_change") and hasattr(password, "min_val"):
            def _show_slider():
                import threading
                req = password
                _pending = [None]

                dlg = tk.Toplevel(self)
                dlg.title(t("fan_slider_title"))
                dlg.resizable(False, False)
                dlg.grab_set()
                dlg.transient(self)

                w, h = 500, 230
                sw, sh = dlg.winfo_screenwidth(), dlg.winfo_screenheight()
                dlg.geometry(f"{w}x{h}+{(sw-w)//2}+{(sh-h)//2}")

                tk.Label(dlg, text=prompt, font=F_NORMAL,
                         wraplength=460, justify="left",
                         pady=12, padx=20).pack(fill="x")

                val_var = tk.DoubleVar(value=req.initial_val)
                val_lbl = tk.Label(
                    dlg, text=f"{req.initial_val}{req.unit}", font=F_BOLD, pady=4
                )
                val_lbl.pack()

                def _on_change(v):
                    val = int(float(v))
                    val_lbl.config(text=f"{val}{req.unit}")
                    if _pending[0]:
                        dlg.after_cancel(_pending[0])
                    def _fire():
                        threading.Thread(
                            target=req.on_change, args=(val,), daemon=True
                        ).start()
                    _pending[0] = dlg.after(250, _fire)

                ttk.Scale(
                    dlg, from_=req.min_val, to=req.max_val,
                    variable=val_var, orient="horizontal",
                    command=_on_change, length=420,
                ).pack(padx=40, pady=(0, 8))

                bf = tk.Frame(dlg)
                bf.pack(pady=(0, 16))

                def _pass():
                    self._manual_input_result = "pass"
                    dlg.destroy()
                    self._manual_event.set()

                def _fail():
                    self._manual_input_result = "fail"
                    dlg.destroy()
                    self._manual_event.set()

                tk.Button(bf, text=t("manual_pass"), bg=COLOR_PASS, fg="white",
                          font=F_BOLD, padx=20, pady=6, relief="flat", cursor="hand2",
                          command=_pass).pack(side="left", padx=10)
                tk.Button(bf, text=t("manual_fail"), bg=COLOR_FAIL, fg="white",
                          font=F_BOLD, padx=20, pady=6, relief="flat", cursor="hand2",
                          command=_fail).pack(side="left", padx=10)

                dlg.protocol("WM_DELETE_WINDOW", _fail)

            self.after(0, _show_slider)
            self._manual_event.wait()
            return self._manual_input_result

        # ── 图片对比模式（password 传入 bytes 时触发）──────────────────
        if isinstance(password, (bytes, bytearray)):
            def _show_image():
                import io, tempfile, os
                tmp_path = None
                try:
                    from PIL import Image
                    img = Image.open(io.BytesIO(password))
                    # 缩放到最大 800×450
                    max_w, max_h = 800, 450
                    scale = min(max_w / img.width, max_h / img.height)
                    new_size = (int(img.width * scale), int(img.height * scale))
                    img = img.resize(new_size, Image.LANCZOS)
                    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as f:
                        tmp_path = f.name
                    img.save(tmp_path)
                    photo = tk.PhotoImage(file=tmp_path)
                except Exception:
                    if tmp_path and os.path.exists(tmp_path):
                        os.unlink(tmp_path)
                    self._manual_input_result = None
                    self._manual_event.set()
                    return

                dlg = tk.Toplevel(self)
                dlg.title(t("hdmi_compare_title"))
                dlg.resizable(True, True)
                dlg.grab_set()
                dlg.transient(self)

                win_w = new_size[0] + 40
                win_h = new_size[1] + 120
                sw, sh = dlg.winfo_screenwidth(), dlg.winfo_screenheight()
                dlg.geometry(f"{win_w}x{win_h}+{(sw-win_w)//2}+{(sh-win_h)//2}")

                img_lbl = tk.Label(dlg, image=photo, bd=1, relief="solid")
                img_lbl.image = photo  # 防止 GC
                img_lbl.pack(padx=20, pady=(16, 8))

                tk.Label(
                    dlg, text=prompt, font=F_NORMAL,
                    wraplength=win_w - 40, justify="center",
                ).pack(pady=(0, 8))

                bf = tk.Frame(dlg)
                bf.pack(pady=(0, 16))

                def _cleanup():
                    if tmp_path and os.path.exists(tmp_path):
                        os.unlink(tmp_path)

                def _match():
                    self._manual_input_result = "pass"
                    dlg.destroy()
                    _cleanup()
                    self._manual_event.set()

                def _no_match():
                    self._manual_input_result = "fail"
                    dlg.destroy()
                    _cleanup()
                    self._manual_event.set()

                tk.Button(
                    bf, text=t("hdmi_match"), bg=COLOR_PASS, fg="white",
                    font=F_BOLD, padx=20, pady=6, relief="flat", cursor="hand2",
                    command=_match,
                ).pack(side="left", padx=10)
                tk.Button(
                    bf, text=t("hdmi_no_match"), bg=COLOR_FAIL, fg="white",
                    font=F_BOLD, padx=20, pady=6, relief="flat", cursor="hand2",
                    command=_no_match,
                ).pack(side="left", padx=10)

                dlg.protocol("WM_DELETE_WINDOW", _no_match)

            self.after(0, _show_image)
            self._manual_event.wait()
            return self._manual_input_result

        def _show():
            dlg = tk.Toplevel(self)
            dlg.title(t("manual_input_title"))
            dlg.resizable(False, False)
            dlg.grab_set()
            dlg.transient(self)

            w, h = 460, 260 if choices else 200
            sw, sh = dlg.winfo_screenwidth(), dlg.winfo_screenheight()
            dlg.geometry(f"{w}x{h}+{(sw-w)//2}+{(sh-h)//2}")

            tk.Label(dlg, text=prompt, font=F_NORMAL,
                     wraplength=420, justify="left",
                     pady=12, padx=20).pack(fill="x")

            var = tk.StringVar()

            if choices:
                import tkinter.ttk as ttk
                combo = ttk.Combobox(dlg, textvariable=var, values=choices,
                                     state="readonly", font=F_NORMAL, width=40)
                if choices:
                    combo.current(0)
                combo.pack(padx=20, pady=(0, 8))

                pw_var = tk.StringVar()
                tk.Label(dlg, text=t("wifi_password_label"), font=F_NORMAL,
                         anchor="w", padx=20).pack(fill="x")
                pw_entry = tk.Entry(dlg, textvariable=pw_var, show="*",
                                    font=F_NORMAL, width=42)
                pw_entry.pack(padx=20, pady=(0, 8))
                pw_entry.focus_set()
            else:
                entry = tk.Entry(dlg, textvariable=var, font=F_NORMAL, width=42,
                                 show="*" if password else "")
                entry.pack(padx=20, pady=(0, 8))
                entry.focus_set()

            bf = tk.Frame(dlg)
            bf.pack(pady=(4, 14))

            def _ok():
                if choices:
                    self._manual_input_result = f"{var.get()}\n{pw_var.get()}"
                else:
                    self._manual_input_result = var.get()
                dlg.destroy()
                self._manual_event.set()

            def _cancel():
                self._manual_input_result = None
                dlg.destroy()
                self._manual_event.set()

            tk.Button(bf, text=t("btn_ok"), bg=COLOR_PASS, fg="white",
                      font=F_BOLD, padx=20, pady=6, relief="flat",
                      cursor="hand2", command=_ok).pack(side="left", padx=10)
            tk.Button(bf, text=t("btn_cancel"), bg="#95a5a6", fg="white",
                      font=F_BOLD, padx=20, pady=6, relief="flat",
                      cursor="hand2", command=_cancel).pack(side="left", padx=10)

            dlg.protocol("WM_DELETE_WINDOW", _cancel)
            dlg.bind("<Return>", lambda e: _ok())
            dlg.bind("<Escape>", lambda e: _cancel())

        self.after(0, _show)
        self._manual_event.wait()
        return self._manual_input_result

    # ------------------------------------------------------------------
    def _on_save_report(self) -> None:
        if self._reporter is None:
            messagebox.showwarning(t("save_fail"), t("no_reporter"), parent=self)
            return

        os.makedirs(REPORT_DIR, exist_ok=True)
        default_name = datetime.now().strftime("report_%Y%m%d_%H%M%S.html")
        filepath = filedialog.asksaveasfilename(
            parent=self,
            title=t("save_report_title"),
            initialdir=os.path.abspath(REPORT_DIR),
            initialfile=default_name,
            defaultextension=".html",
            filetypes=[("HTML", "*.html"), ("*", "*.*")],
        )
        if not filepath:
            return

        try:
            self._reporter.save(filepath)
        except Exception as exc:
            messagebox.showerror(t("save_fail"), str(exc), parent=self)
            return

        if messagebox.askyesno(
            t("save_success_title"), t("save_success_msg", filepath), parent=self
        ):
            webbrowser.open(filepath)

    def _on_retry(self) -> None:
        self.app.show_page("select")
