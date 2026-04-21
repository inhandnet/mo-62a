"""第1页：连接配置"""
import threading
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog

from config import DEFAULT_PASSWORD, DEFAULT_SSH_USER, DEFAULT_SSH_PORT
from gui.i18n import t
from gui.fonts import F_NORMAL, F_BOLD, F_SMALL, F_TITLE, F_HEADER, F_CODE

COLOR_PRIMARY = "#2c5f9e"
COLOR_BG      = "#f5f7fa"


class ConnectPage(tk.Frame):

    def __init__(self, parent: tk.Widget, app) -> None:
        super().__init__(parent, bg=COLOR_BG)
        self.app = app
        self._mode = tk.StringVar(value="ssh")
        self._build_ui()
        app.register_lang_callback(self.update_language)

    # ------------------------------------------------------------------
    def _build_ui(self) -> None:
        self.columnconfigure(0, weight=1)

        # ── 主体 ────────────────────────────────────────────────────────
        body = tk.Frame(self, bg=COLOR_BG)
        body.grid(row=0, column=0, sticky="new", padx=30, pady=20)
        body.columnconfigure(0, weight=1)

        row = 0

        # ── 语言选择（LabelFrame）──────────────────────────────────────
        self._lang_frame = tk.LabelFrame(
            body, text=t("lang_frame"), bg=COLOR_BG, font=F_NORMAL,
            padx=10, pady=6,
        )
        self._lang_frame.grid(row=row, column=0, sticky="ew", pady=(0, 10))
        row += 1

        self._lang_btn = tk.Button(
            self._lang_frame, text=t("lang_toggle"),
            font=F_SMALL, padx=12, pady=3, relief="groove", cursor="hand2",
            command=self.app._toggle_lang,
        )
        self._lang_btn.pack(side="left")

        # ── 连接方式（LabelFrame）──────────────────────────────────────
        self._conn_mode_frame = tk.LabelFrame(
            body, text=t("conn_mode_frame"), bg=COLOR_BG, font=F_NORMAL,
            padx=10, pady=10,
        )
        self._conn_mode_frame.grid(row=row, column=0, sticky="ew", pady=(0, 10))
        row += 1

        self._rb_ssh = tk.Radiobutton(
            self._conn_mode_frame, text=t("conn_ssh"),
            variable=self._mode, value="ssh",
            bg=COLOR_BG, font=F_NORMAL, command=self._on_mode_change,
        )
        self._rb_ssh.pack(side="left", padx=(0, 20))

        self._rb_serial = tk.Radiobutton(
            self._conn_mode_frame, text=t("conn_serial"),
            variable=self._mode, value="serial",
            bg=COLOR_BG, font=F_NORMAL, command=self._on_mode_change,
        )
        self._rb_serial.pack(side="left")

        # ── SSH 面板 ────────────────────────────────────────────────────
        self._ssh_frame = tk.LabelFrame(
            body, text=t("ssh_frame_label"), bg=COLOR_BG, font=F_NORMAL,
            padx=10, pady=10,
        )
        self._ssh_frame.grid(row=row, column=0, sticky="ew", pady=(0, 10))
        self._ssh_frame.columnconfigure(0, weight=1)
        row += 1
        self._build_ssh_panel(self._ssh_frame)

        # ── 串口面板 ────────────────────────────────────────────────────
        self._serial_frame = tk.LabelFrame(
            body, text=t("serial_frame_label"), bg=COLOR_BG, font=F_NORMAL,
            padx=10, pady=10,
        )
        self._serial_frame.grid(row=row, column=0, sticky="ew", pady=(0, 10))
        self._serial_frame.columnconfigure(0, weight=1)
        row += 1
        self._build_serial_panel(self._serial_frame)

        # ── 用户名 + 密码 ────────────────────────────────────────────────
        self._cred_frame = tk.LabelFrame(
            body, text=t("cred_frame_label"), bg=COLOR_BG, font=F_NORMAL,
            padx=10, pady=10,
        )
        self._cred_frame.grid(row=row, column=0, sticky="ew", pady=(0, 10))
        row += 1

        self._user_lbl = tk.Label(
            self._cred_frame, text=t("username_label"), bg=COLOR_BG, font=F_NORMAL, anchor="e"
        )
        self._user_lbl.grid(row=0, column=0, sticky="e", padx=(0, 8), pady=(0, 6))

        self._username_var = tk.StringVar(value=DEFAULT_SSH_USER)
        tk.Entry(
            self._cred_frame, textvariable=self._username_var, width=22, font=F_NORMAL,
        ).grid(row=0, column=1, sticky="w", pady=(0, 6))

        self._pwd_lbl = tk.Label(
            self._cred_frame, text=t("password_label"), bg=COLOR_BG, font=F_NORMAL, anchor="e"
        )
        self._pwd_lbl.grid(row=1, column=0, sticky="e", padx=(0, 8))

        self._password_var = tk.StringVar(value=DEFAULT_PASSWORD)
        self._pwd_entry = tk.Entry(
            self._cred_frame, textvariable=self._password_var,
            show="*", width=22, font=F_NORMAL,
        )
        self._pwd_entry.grid(row=1, column=1, sticky="w")

        self._pwd_show = False
        self._eye_btn = tk.Button(
            self._cred_frame, text=t("pwd_show"), font=F_SMALL,
            bg=COLOR_BG, relief="groove", padx=6, pady=2, cursor="hand2",
            command=self._toggle_pwd_show,
        )
        self._eye_btn.grid(row=1, column=2, sticky="w", padx=(6, 0))

        self._pwd_hint_lbl = tk.Label(
            self._cred_frame, text=t("password_hint"), bg=COLOR_BG, fg="#888", font=F_SMALL,
        )
        self._pwd_hint_lbl.grid(row=1, column=3, sticky="w", padx=(8, 0))

        # ── 状态 + 连接按钮 ─────────────────────────────────────────────
        ttk.Separator(body, orient="horizontal").grid(
            row=row, column=0, sticky="ew", pady=8
        )
        row += 1

        bottom = tk.Frame(body, bg=COLOR_BG)
        bottom.grid(row=row, column=0, sticky="ew")
        bottom.columnconfigure(0, weight=1)

        self._status_var = tk.StringVar(value=t("status_ready"))
        tk.Label(
            bottom, textvariable=self._status_var,
            bg=COLOR_BG, fg="#555", font=F_NORMAL, anchor="w",
        ).grid(row=0, column=0, sticky="w")

        self._connect_btn = tk.Button(
            bottom, text=t("connect_btn"),
            bg=COLOR_PRIMARY, fg="white", font=F_BOLD,
            padx=16, pady=6, relief="flat", cursor="hand2",
            command=self._on_connect,
        )
        self._connect_btn.grid(row=0, column=1, sticky="e")

        self._on_mode_change()

    def _toggle_pwd_show(self) -> None:
        self._pwd_show = not self._pwd_show
        self._pwd_entry.config(show="" if self._pwd_show else "*")
        self._eye_btn.config(text=t("pwd_hide") if self._pwd_show else t("pwd_show"))

    def _build_ssh_panel(self, parent: tk.Widget) -> None:
        parent.columnconfigure(0, weight=1)

        tree_frame = tk.Frame(parent, bg=COLOR_BG)
        tree_frame.grid(row=0, column=0, sticky="ew", pady=(0, 8))
        tree_frame.columnconfigure(0, weight=1)

        columns = ("hostname", "ip", "version", "mac")
        self._tree = ttk.Treeview(
            tree_frame, columns=columns, show="headings",
            height=4, selectmode="browse",
        )
        self._tree_cols = [
            ("hostname", "col_hostname", 140),
            ("ip",       "col_ip",      140),
            ("version",  "col_version", 100),
            ("mac",      "col_mac",     140),
        ]
        for col, key, width in self._tree_cols:
            self._tree.heading(col, text=t(key))
            self._tree.column(col, width=width, anchor="center")

        vsb = ttk.Scrollbar(tree_frame, orient="vertical", command=self._tree.yview)
        self._tree.configure(yscrollcommand=vsb.set)
        self._tree.grid(row=0, column=0, sticky="ew")
        vsb.grid(row=0, column=1, sticky="ns")

        self._tree.bind("<Double-1>", self._on_tree_select)
        self._tree.bind("<<TreeviewSelect>>", self._on_tree_select)

        manual_frame = tk.Frame(parent, bg=COLOR_BG)
        manual_frame.grid(row=1, column=0, sticky="w")

        self._manual_ip_lbl = tk.Label(
            manual_frame, text=t("manual_ip"), bg=COLOR_BG, font=F_NORMAL
        )
        self._manual_ip_lbl.pack(side="left", padx=(0, 6))

        self._ip_var = tk.StringVar()
        tk.Entry(
            manual_frame, textvariable=self._ip_var, width=20, font=F_NORMAL
        ).pack(side="left", padx=(0, 6))

        self._scan_btn = tk.Button(
            manual_frame, text=t("scan"), font=F_SMALL,
            padx=10, pady=3, relief="groove", cursor="hand2",
            command=self._on_scan,
        )
        self._scan_btn.pack(side="left")

    def _build_serial_panel(self, parent: tk.Widget) -> None:
        parent.columnconfigure(0, weight=1)

        row_frame = tk.Frame(parent, bg=COLOR_BG)
        row_frame.grid(row=0, column=0, sticky="w")

        self._serial_port_lbl = tk.Label(
            row_frame, text=t("serial_port_label"), bg=COLOR_BG, font=F_NORMAL
        )
        self._serial_port_lbl.pack(side="left", padx=(0, 6))

        self._port_var = tk.StringVar()
        self._port_combo = ttk.Combobox(
            row_frame, textvariable=self._port_var, width=22, font=F_NORMAL
        )
        self._port_combo.pack(side="left", padx=(0, 8))

        self._refresh_btn = tk.Button(
            row_frame, text=t("refresh"), font=F_SMALL,
            padx=10, pady=3, relief="groove", cursor="hand2",
            command=self._refresh_ports,
        )
        self._refresh_btn.pack(side="left")

        self._refresh_ports()

    # ------------------------------------------------------------------
    # 语言切换
    # ------------------------------------------------------------------
    def update_language(self) -> None:
        self._lang_frame.config(text=t("lang_frame"))
        self._lang_btn.config(text=t("lang_toggle"))
        self._conn_mode_frame.config(text=t("conn_mode_frame"))
        self._rb_ssh.config(text=t("conn_ssh"))
        self._rb_serial.config(text=t("conn_serial"))
        self._ssh_frame.config(text=t("ssh_frame_label"))
        self._serial_frame.config(text=t("serial_frame_label"))
        self._scan_btn.config(text=t("scan"))
        self._manual_ip_lbl.config(text=t("manual_ip"))
        self._serial_port_lbl.config(text=t("serial_port_label"))
        self._refresh_btn.config(text=t("refresh"))
        self._cred_frame.config(text=t("cred_frame_label"))
        self._user_lbl.config(text=t("username_label"))
        self._pwd_lbl.config(text=t("password_label"))
        self._eye_btn.config(text=t("pwd_hide") if self._pwd_show else t("pwd_show"))
        self._pwd_hint_lbl.config(text=t("password_hint"))
        self._connect_btn.config(text=t("connect_btn"))
        # 更新 Treeview 列头
        for col, key, _ in self._tree_cols:
            self._tree.heading(col, text=t(key))
        # 状态只在就绪时才更新（避免覆盖进行中的提示）
        if self._status_var.get() in ("就绪", "Ready"):
            self._status_var.set(t("status_ready"))

    # ------------------------------------------------------------------
    # 事件处理
    # ------------------------------------------------------------------
    def _on_mode_change(self) -> None:
        if self._mode.get() == "ssh":
            self._ssh_frame.grid()
            self._serial_frame.grid_remove()
        else:
            self._ssh_frame.grid_remove()
            self._serial_frame.grid()

    def _on_tree_select(self, event=None) -> None:
        sel = self._tree.selection()
        if not sel:
            return
        values = self._tree.item(sel[0], "values")
        if values:
            self._ip_var.set(values[1])

    def _on_scan(self) -> None:
        self._scan_btn.config(text=t("scanning"), state="disabled")
        self._status_var.set(t("scanning"))
        self._tree.delete(*self._tree.get_children())

        def _worker():
            try:
                from framework.discovery import discover_devices
                devices = discover_devices()
            except Exception as exc:
                devices = []
                self.after(0, lambda: self._status_var.set(t("scan_fail", exc)))

            def _update():
                self._tree.delete(*self._tree.get_children())
                for d in devices:
                    self._tree.insert("", "end", values=(
                        d.get("hostname", ""),
                        d.get("ip", ""),
                        d.get("version", ""),
                        d.get("mac", ""),
                    ))
                self._scan_btn.config(text=t("scan"), state="normal")
                if devices:
                    self._status_var.set(t("scan_done", len(devices)))
                else:
                    self._status_var.set(t("scan_none"))

            self.after(0, _update)

        threading.Thread(target=_worker, daemon=True).start()

    def _refresh_ports(self) -> None:
        try:
            from framework.serial_conn import list_ports
            ports = list_ports()
        except Exception:
            ports = []
        values = [f"{p} - {d}" if d else p for p, d in ports]
        self._port_combo["values"] = values
        if values:
            self._port_combo.current(0)

    def _on_connect(self) -> None:
        self._connect_btn.config(state="disabled")
        self._status_var.set(t("connecting"))

        def _worker():
            mode = self._mode.get()
            user_password = self._password_var.get().strip()
            try:
                if mode == "ssh":
                    self._do_ssh_connect(user_password)
                else:
                    self._do_serial_connect(user_password)
            except Exception as exc:
                self.after(0, lambda e=exc: self._on_connect_error(str(e)))

        threading.Thread(target=_worker, daemon=True).start()

    def _do_ssh_connect(self, user_password: str) -> None:
        from framework.board import Board

        host = self._ip_var.get().strip()
        if not host:
            self.after(0, lambda: self._on_connect_error(t("no_ip")))
            return

        username = self._username_var.get().strip() or DEFAULT_SSH_USER
        password = user_password if user_password else DEFAULT_PASSWORD
        board = Board()

        try:
            board.connect(host, username, password)
        except Exception as exc:
            self.after(0, lambda e=exc: self._on_connect_error(t("conn_fail", e)))
            return

        if password == DEFAULT_PASSWORD:
            def _prompt_new_password():
                new_pwd = simpledialog.askstring(
                    t("change_pwd_title"), t("change_pwd_prompt"),
                    show="*", parent=self,
                )
                if new_pwd:
                    try:
                        board.change_password(new_pwd)
                        messagebox.showinfo(
                            t("pwd_updated_title"), t("pwd_updated_msg"), parent=self
                        )
                    except Exception as exc:
                        messagebox.showwarning(
                            t("pwd_fail_title"), t("pwd_fail_msg", exc), parent=self
                        )
                self._finish_connect(board, host)

            self.after(0, _prompt_new_password)
        else:
            self.after(0, lambda: self._finish_connect(board, host))

    def _do_serial_connect(self, user_password: str) -> None:
        from framework.serial_conn import SerialBoard

        port_text = self._port_var.get().strip()
        if not port_text:
            self.after(0, lambda: self._on_connect_error(t("no_port")))
            return

        port = port_text.split(" - ")[0].strip()
        password = user_password if user_password else DEFAULT_PASSWORD
        board = SerialBoard()
        try:
            board.connect(port, password)
        except Exception as exc:
            self.after(0, lambda e=exc: self._on_connect_error(t("serial_fail", e)))
            return

        self.after(0, lambda: self._finish_connect(board, port))

    def _finish_connect(self, board, host: str) -> None:
        self.app.board = board
        info = {"ip": host, "hostname": "N/A", "version": "N/A",
                "build_date": "N/A", "test_time": "N/A"}
        try:
            _, out, _ = board.run("hostname 2>/dev/null")
            info["hostname"] = out.strip() or "N/A"
        except Exception:
            pass
        try:
            _, out, _ = board.run("mo-version 2>/dev/null")
            for line in out.splitlines():
                line = line.strip()
                if line.startswith("MO-"):
                    info["version"] = line
                elif line.startswith("Built:"):
                    info["build_date"] = line.split(":", 1)[1].strip()
        except Exception:
            pass
        self.app.device_info = info
        self._status_var.set(t("conn_success"))
        self._connect_btn.config(state="normal")
        self.app.show_page("select")

    def _on_connect_error(self, msg: str) -> None:
        self._status_var.set(t("conn_fail", msg))
        self._connect_btn.config(state="normal")
        messagebox.showerror(t("conn_fail", ""), msg, parent=self)
