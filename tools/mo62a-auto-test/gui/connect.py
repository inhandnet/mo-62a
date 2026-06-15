"""连接页 — 设备发现 + SSH 连接配置 + 历史记录"""

from __future__ import annotations
import sys
import platform

import PySide6
from PySide6.QtCore import Qt, Signal, QThread
from PySide6.QtGui import QPixmap, QPainter, QColor, QPen, QFont
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QLineEdit, QListWidget, QListWidgetItem, QFrame,
    QSizePolicy, QGraphicsDropShadowEffect, QAbstractItemView,
)

from config.settings import APP_VERSION
from config.i18n import t

# ── 颜色（与 welcome.py 保持一致）────────────────────────────────────────────
C_BG      = "#0d1117"
C_BAR     = "#161b22"
C_BORDER  = "#21262d"
C_ACCENT  = "#00d4ff"
C_ACCENT2 = "#0ea5e9"
C_TEXT    = "#e6edf3"
C_MUTED   = "#8b949e"
C_CARD    = "#161b22"
C_CARD2   = "#1c2333"
C_GREEN   = "#3fb950"
C_RED     = "#f85149"


# ── 网格背景（同 welcome.py）─────────────────────────────────────────────────
class _GridWidget(QWidget):
    def paintEvent(self, event):
        p = QPainter(self)
        p.fillRect(self.rect(), QColor(C_BG))
        pen = QPen(QColor("#161b22"))
        pen.setWidth(1)
        p.setPen(pen)
        step = 40
        for x in range(0, self.width() + step, step):
            p.drawLine(x, 0, x, self.height())
        for y in range(0, self.height() + step, step):
            p.drawLine(0, y, self.width(), y)
        p.setPen(Qt.PenStyle.NoPen)
        p.setBrush(QColor("#1c2333"))
        for x in range(0, self.width() + step, step):
            for y in range(0, self.height() + step, step):
                p.drawEllipse(x - 1, y - 1, 3, 3)
        p.end()


# ── 后台线程：设备扫描 ────────────────────────────────────────────────────────
class _ScanThread(QThread):
    result = Signal(list)
    error  = Signal(str)

    def run(self):
        try:
            from connection.discovery import discover
            self.result.emit(discover())
        except Exception as e:
            self.error.emit(str(e))


# ── 后台线程：SSH 连接 ────────────────────────────────────────────────────────
class _ConnectThread(QThread):
    success = Signal(object, str)   # (SSHBoard, hostname)
    error   = Signal(str)

    def __init__(self, ip: str, user: str, password: str):
        super().__init__()
        self._ip       = ip
        self._user     = user
        self._password = password

    def run(self):
        from connection.ssh import SSHBoard, SSHError
        board = SSHBoard()
        try:
            board.connect(self._ip, self._user, self._password)
            _, hostname, _ = board.run("hostname 2>/dev/null || echo unknown")
            self.success.emit(board, hostname.strip())
        except SSHError as e:
            self.error.emit(str(e))
        except Exception as e:
            self.error.emit(str(e))


# ── 连接页 ────────────────────────────────────────────────────────────────────
class ConnectPage(QWidget):
    back_requested    = Signal()
    connect_succeeded = Signal(object, dict)   # (SSHBoard, device_info)

    def __init__(self, app=None, parent: QWidget | None = None):
        super().__init__(parent)
        self._app         = app
        self._board       = None
        self._device_info = {}
        self._scan_thread    : _ScanThread    | None = None
        self._connect_thread : _ConnectThread | None = None
        self._build_ui()
        self._load_history()
        self._apply_lang()

    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    # UI 构建
    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    def _build_ui(self):
        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)
        root.addWidget(self._make_topbar())
        root.addWidget(self._make_content(), stretch=1)
        root.addWidget(self._make_footer())

    # ── 顶栏 ─────────────────────────────────────────────────────────────────
    def _make_topbar(self) -> QWidget:
        bar = QWidget()
        bar.setFixedHeight(50)
        bar.setStyleSheet(f"background:{C_BAR}; border-bottom:1px solid {C_BORDER};")
        lay = QHBoxLayout(bar)
        lay.setContentsMargins(24, 0, 24, 0)

        brand = QLabel("◈  Mo 62A Auto Test")
        brand.setStyleSheet(
            f"color:{C_ACCENT}; font-size:14px; font-weight:bold;"
            "font-family:'Courier New','Consolas',monospace;"
            "background:transparent; letter-spacing:1px;"
        )
        lay.addWidget(brand)
        lay.addStretch()
        return bar

    # ── 主内容（双栏）────────────────────────────────────────────────────────
    def _make_content(self) -> QWidget:
        bg = _GridWidget()
        lay = QHBoxLayout(bg)
        lay.setContentsMargins(28, 24, 28, 24)
        lay.setSpacing(20)
        lay.addWidget(self._make_left_panel(),  stretch=4)
        lay.addWidget(self._make_right_panel(), stretch=5)
        return bg

    # ── 左栏：设备发现 ────────────────────────────────────────────────────────
    def _make_left_panel(self) -> QWidget:
        card = self._card_widget()
        lay  = QVBoxLayout(card)
        lay.setContentsMargins(16, 14, 16, 14)
        lay.setSpacing(10)

        # 标题行
        title_row = QHBoxLayout()
        self._disc_title = QLabel()
        self._disc_title.setStyleSheet(
            f"color:{C_TEXT}; font-size:13px; font-weight:bold; background:transparent;"
        )
        title_row.addWidget(self._disc_title)
        title_row.addStretch()

        self._scan_btn = QPushButton()
        self._scan_btn.setFixedHeight(30)
        self._scan_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self._scan_btn.setStyleSheet(f"""
            QPushButton {{
                color:{C_ACCENT}; background:transparent;
                border:1px solid {C_ACCENT}; border-radius:5px;
                font-size:12px; padding:0 14px;
            }}
            QPushButton:hover {{ background:{C_ACCENT}22; }}
            QPushButton:disabled {{ color:{C_MUTED}; border-color:#30363d; }}
        """)
        self._scan_btn.clicked.connect(self._on_scan)
        title_row.addWidget(self._scan_btn)
        lay.addLayout(title_row)

        # 状态标签
        self._scan_status = QLabel("")
        self._scan_status.setStyleSheet(
            f"color:{C_MUTED}; font-size:11px; background:transparent;"
        )
        lay.addWidget(self._scan_status)

        # 设备列表
        self._device_list = QListWidget()
        self._device_list.setStyleSheet(f"""
            QListWidget {{
                background:{C_BG}; border:1px solid {C_BORDER};
                border-radius:6px; color:{C_TEXT}; font-size:12px;
                outline:none;
            }}
            QListWidget::item {{
                padding:8px 10px; border-bottom:1px solid {C_BORDER};
            }}
            QListWidget::item:selected {{
                background:{C_ACCENT}22; color:{C_ACCENT};
            }}
            QListWidget::item:hover {{ background:{C_CARD2}; }}
        """)
        self._device_list.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)
        self._device_list.itemClicked.connect(self._on_device_clicked)
        lay.addWidget(self._device_list, stretch=1)

        return card

    # ── 右栏：连接表单 + 历史 ────────────────────────────────────────────────
    def _make_right_panel(self) -> QWidget:
        card = self._card_widget()
        lay  = QVBoxLayout(card)
        lay.setContentsMargins(16, 14, 16, 14)
        lay.setSpacing(0)

        # —— 连接配置区 ——
        self._form_title = QLabel()
        self._form_title.setStyleSheet(
            f"color:{C_TEXT}; font-size:13px; font-weight:bold; background:transparent;"
        )
        lay.addWidget(self._form_title)
        lay.addSpacing(12)

        # 表单
        form_grid = QVBoxLayout()
        form_grid.setSpacing(8)

        self._ip_label   = QLabel(); self._ip_input   = self._input_field()
        self._user_label = QLabel(); self._user_input = self._input_field()
        self._pwd_label  = QLabel()

        # 密码行（含显示/隐藏按钮）
        pwd_row = QHBoxLayout()
        pwd_row.setSpacing(6)
        self._pwd_input = self._input_field(password=True)
        self._eye_btn   = QPushButton("◎")
        self._eye_btn.setFixedSize(32, 34)
        self._eye_btn.setCheckable(True)
        self._eye_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self._eye_btn.setStyleSheet(f"""
            QPushButton {{
                color:{C_MUTED}; background:transparent;
                border:1px solid {C_BORDER}; border-radius:5px; font-size:14px;
            }}
            QPushButton:checked {{ color:{C_ACCENT}; border-color:{C_ACCENT}; }}
            QPushButton:hover   {{ color:{C_TEXT}; }}
        """)
        self._eye_btn.toggled.connect(
            lambda v: self._pwd_input.setEchoMode(
                QLineEdit.EchoMode.Normal if v else QLineEdit.EchoMode.Password
            )
        )
        pwd_row.addWidget(self._pwd_input)
        pwd_row.addWidget(self._eye_btn)

        for lbl, widget in [
            (self._ip_label,   self._ip_input),
            (self._user_label, self._user_input),
            (self._pwd_label,  None),
        ]:
            lbl.setStyleSheet(
                f"color:{C_MUTED}; font-size:11px; background:transparent;"
            )
            form_grid.addWidget(lbl)
            if widget:
                form_grid.addWidget(widget)
            else:
                form_grid.addLayout(pwd_row)

        lay.addLayout(form_grid)
        lay.addSpacing(8)

        # 连接状态标签（仅显示错误信息）
        self._conn_status = QLabel("")
        self._conn_status.setWordWrap(True)
        self._conn_status.setStyleSheet(
            f"color:{C_MUTED}; font-size:12px; background:transparent;"
        )
        lay.addWidget(self._conn_status)

        # 分隔线
        lay.addSpacing(14)
        sep = QFrame()
        sep.setFrameShape(QFrame.Shape.HLine)
        sep.setStyleSheet(f"color:{C_BORDER}; background:{C_BORDER};")
        lay.addWidget(sep)
        lay.addSpacing(10)

        # —— 历史记录区 ——
        self._hist_title = QLabel()
        self._hist_title.setStyleSheet(
            f"color:{C_TEXT}; font-size:13px; font-weight:bold; background:transparent;"
        )
        lay.addWidget(self._hist_title)
        lay.addSpacing(8)

        self._history_list = QListWidget()
        self._history_list.setStyleSheet(f"""
            QListWidget {{
                background:{C_BG}; border:1px solid {C_BORDER};
                border-radius:6px; color:{C_TEXT}; font-size:12px;
                outline:none;
            }}
            QListWidget::item {{
                padding:7px 10px; border-bottom:1px solid {C_BORDER};
            }}
            QListWidget::item:selected {{
                background:{C_ACCENT}22; color:{C_ACCENT};
            }}
            QListWidget::item:hover {{ background:{C_CARD2}; }}
        """)
        self._history_list.setSelectionMode(QAbstractItemView.SelectionMode.SingleSelection)
        self._history_list.itemClicked.connect(self._on_history_clicked)
        self._history_list.itemDoubleClicked.connect(self._on_history_double_clicked)
        lay.addWidget(self._history_list, stretch=1)

        return card

    # ── 底栏：返回 / 继续 ─────────────────────────────────────────────────────
    def _make_footer(self) -> QWidget:
        bar = QWidget()
        bar.setFixedHeight(52)
        bar.setStyleSheet(f"background:{C_BAR}; border-top:1px solid {C_BORDER};")
        lay = QHBoxLayout(bar)
        lay.setContentsMargins(24, 0, 24, 0)

        self._back_btn = self._nav_btn("", primary=False)
        self._back_btn.clicked.connect(self.back_requested)
        lay.addWidget(self._back_btn)

        lay.addStretch()

        py_ver  = f"Python {sys.version_info.major}.{sys.version_info.minor}"
        ps_ver  = f"PySide6 {PySide6.__version__}"
        os_info = f"{platform.system()} {platform.machine()}"
        info    = QLabel(f"v{APP_VERSION}  ·  {py_ver}  ·  {ps_ver}  ·  {os_info}")
        info.setStyleSheet(f"color:{C_MUTED}; font-size:11px; background:transparent;")
        lay.addWidget(info)

        lay.addStretch()

        self._continue_btn = self._nav_btn("", primary=True)
        self._continue_btn.setEnabled(False)
        self._continue_btn.clicked.connect(self._on_continue)
        lay.addWidget(self._continue_btn)

        return bar

    # ── 工具方法 ──────────────────────────────────────────────────────────────
    def _card_widget(self) -> QWidget:
        w = QWidget()
        w.setStyleSheet(
            f"background:{C_CARD}; border-radius:10px; border:1px solid {C_BORDER};"
        )
        return w

    def _input_field(self, default: str = "", password: bool = False) -> QLineEdit:
        e = QLineEdit(default)
        e.setFixedHeight(34)
        if password:
            e.setEchoMode(QLineEdit.EchoMode.Password)
        e.setStyleSheet(f"""
            QLineEdit {{
                background:{C_BG}; color:{C_TEXT};
                border:1px solid {C_BORDER}; border-radius:5px;
                padding:0 10px; font-size:13px;
            }}
            QLineEdit:focus {{ border-color:{C_ACCENT}; }}
        """)
        # 任何字段改动都重置连接成功状态
        e.textChanged.connect(self._on_field_changed)
        return e

    def _nav_btn(self, text: str, primary: bool) -> QPushButton:
        btn = QPushButton(text)
        btn.setFixedSize(100, 34)
        btn.setCursor(Qt.CursorShape.PointingHandCursor)
        if primary:
            btn.setStyleSheet(f"""
                QPushButton {{
                    background:qlineargradient(x1:0,y1:0,x2:1,y2:0,
                        stop:0 {C_ACCENT2},stop:1 {C_ACCENT});
                    color:#0d1117; border:none; border-radius:6px;
                    font-size:13px; font-weight:bold;
                }}
                QPushButton:hover  {{ background:{C_ACCENT}; }}
                QPushButton:pressed {{ background:#0284c7; }}
                QPushButton:disabled {{
                    background:#21262d; color:{C_MUTED};
                }}
            """)
        else:
            btn.setStyleSheet(f"""
                QPushButton {{
                    color:{C_TEXT}; background:transparent;
                    border:1px solid {C_BORDER}; border-radius:6px;
                    font-size:13px;
                }}
                QPushButton:hover  {{ border-color:{C_ACCENT}; color:{C_ACCENT}; }}
                QPushButton:pressed {{ background:{C_CARD2}; }}
            """)
        return btn

    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    # 事件处理
    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    def _on_scan(self):
        self._scan_btn.setEnabled(False)
        self._device_list.clear()
        self._scan_status.setText(t("conn_scanning"))
        self._scan_status.setStyleSheet(
            f"color:{C_MUTED}; font-size:11px; background:transparent;"
        )
        self._scan_thread = _ScanThread(self)
        self._scan_thread.result.connect(self._on_scan_result)
        self._scan_thread.error.connect(self._on_scan_error)
        self._scan_thread.start()

    def _on_scan_result(self, devices: list):
        self._scan_btn.setEnabled(True)
        if not devices:
            self._scan_status.setText(t("conn_scan_none"))
            return
        self._scan_status.setText(t("conn_scan_found", len(devices)))
        self._scan_status.setStyleSheet(
            f"color:{C_GREEN}; font-size:11px; background:transparent;"
        )
        for d in devices:
            host = d.get("hostname", "") or d["ip"]
            ver  = d.get("version", "")
            line = f"{host}  —  {d['ip']}" + (f"  [{ver}]" if ver else "")
            item = QListWidgetItem(line)
            item.setData(Qt.ItemDataRole.UserRole, d)
            self._device_list.addItem(item)

    def _on_scan_error(self, msg: str):
        self._scan_btn.setEnabled(True)
        self._scan_status.setText(t("conn_scan_error", msg))
        self._scan_status.setStyleSheet(
            f"color:{C_RED}; font-size:11px; background:transparent;"
        )

    def _on_device_clicked(self, item: QListWidgetItem):
        d = item.data(Qt.ItemDataRole.UserRole)
        if d:
            self._ip_input.setText(d.get("ip", ""))

    def _on_history_clicked(self, item: QListWidgetItem):
        d = item.data(Qt.ItemDataRole.UserRole)
        if d:
            self._ip_input.setText(d.get("ip", ""))
            self._user_input.setText(d.get("username", ""))
            self._pwd_input.setText(d.get("password", ""))

    def _on_history_double_clicked(self, item: QListWidgetItem):
        """双击历史记录：填充字段并直接触发连接。"""
        d = item.data(Qt.ItemDataRole.UserRole)
        if not d:
            return
        self._ip_input.setText(d.get("ip", ""))
        self._user_input.setText(d.get("username", ""))
        self._pwd_input.setText(d.get("password", ""))
        # 字段填充后继续按钮已启用，直接触发连接
        if self._continue_btn.isEnabled():
            self._on_continue()

    def _on_field_changed(self):
        """任意字段变化：三项均非空则启用继续按钮，并清除错误提示。"""
        all_filled = bool(
            self._ip_input.text().strip()
            and self._user_input.text().strip()
        )
        self._continue_btn.setEnabled(all_filled)
        self._conn_status.setText("")

    def _on_connect_success(self, board, hostname: str):
        ip   = self._ip_input.text().strip()
        user = self._user_input.text().strip()
        pwd  = self._pwd_input.text()

        # 连接成功后写入历史
        from connection.history import save as history_save
        history_save(ip, user, pwd, hostname)
        self._load_history()

        self._board       = board
        self._device_info = {"ip": ip, "hostname": hostname, "username": user}

        # 直接跳转下一页
        self.connect_succeeded.emit(board, self._device_info)

    def _on_connect_error(self, msg: str):
        # 恢复继续按钮（字段仍然填写着）
        all_filled = bool(
            self._ip_input.text().strip()
            and self._user_input.text().strip()
        )
        self._continue_btn.setEnabled(all_filled)
        self._conn_status.setText(t("conn_failed", msg))
        self._conn_status.setStyleSheet(
            f"color:{C_RED}; font-size:12px; background:transparent;"
        )

    def _on_continue(self):
        ip   = self._ip_input.text().strip()
        user = self._user_input.text().strip()
        pwd  = self._pwd_input.text()

        self._continue_btn.setEnabled(False)
        self._conn_status.setText(t("conn_connecting"))
        self._conn_status.setStyleSheet(
            f"color:{C_MUTED}; font-size:12px; background:transparent;"
        )

        self._connect_thread = _ConnectThread(ip, user, pwd)
        self._connect_thread.success.connect(self._on_connect_success)
        self._connect_thread.error.connect(self._on_connect_error)
        self._connect_thread.start()

    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    # 历史记录
    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    def _load_history(self):
        from connection.history import load as history_load
        self._history_list.clear()
        entries = history_load()
        if not entries:
            placeholder = QListWidgetItem(t("conn_history_empty"))
            placeholder.setFlags(Qt.ItemFlag.NoItemFlags)
            placeholder.setForeground(QColor(C_MUTED))
            self._history_list.addItem(placeholder)
            return
        for e in entries:
            pwd  = e.get("password", "") or t("conn_no_password")
            last = e.get("last_used", "")
            prefix = f"{last}  " if last else ""
            line = f"{prefix}{e['ip']}  ·  {e['username']}  ·  {pwd}"
            item = QListWidgetItem(line)
            item.setData(Qt.ItemDataRole.UserRole, e)
            self._history_list.addItem(item)

    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    # 语言切换
    # ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    def _set_lang(self, lang: str):
        if self._app:
            self._app.apply_lang_all(lang)
        else:
            set_lang(lang)
            self._apply_lang()

    def apply_lang(self):
        """供 MainWindow.apply_lang_all() 调用。"""
        self._apply_lang()
        self._load_history()

    def _apply_lang(self):
        self._disc_title.setText(t("conn_discovery_title"))
        self._scan_btn.setText(t("conn_scan_btn"))
        self._form_title.setText(t("conn_form_title"))
        self._ip_label.setText(t("conn_ip_label"))
        self._user_label.setText(t("conn_user_label"))
        self._pwd_label.setText(t("conn_pwd_label"))
        self._hist_title.setText(t("conn_history_title"))
        self._back_btn.setText(t("btn_back"))
        self._continue_btn.setText(t("btn_continue"))

    def on_show(self):
        """每次切换到此页时刷新历史列表。"""
        self._load_history()
