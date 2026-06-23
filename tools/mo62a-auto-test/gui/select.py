"""测试项选择页"""

from __future__ import annotations
from pathlib import Path

from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QPainter, QColor, QPen
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QTreeWidget, QTreeWidgetItem, QSizePolicy, QFrame,
)

from config.i18n import t

# ── 颜色（与其他页保持一致）──────────────────────────────────────────────────
C_BG     = "#0d1117"
C_BAR    = "#161b22"
C_BORDER = "#21262d"
C_ACCENT = "#00d4ff"
C_ACCENT2= "#0ea5e9"
C_TEXT   = "#e6edf3"
C_MUTED  = "#8b949e"
C_CARD   = "#161b22"
C_CARD2  = "#1c2333"

# ── Checkbox SVG 图标路径 ────────────────────────────────────────────────────
_ASSETS  = Path(__file__).parent / "assets"
_CHECK_SVG = str(_ASSETS / "check.svg").replace("\\", "/")
_DASH_SVG  = str(_ASSETS / "dash.svg").replace("\\", "/")

# ── 测试项定义（category_key → [(name_key, default_checked[, disabled]), ...]）
_TEST_TREE: list[tuple[str, list]] = [
    ("cat_system", [
        ("tn_firmware_version", True),
        ("tn_kernel_version",   True),
        ("tn_cpu_cores",        True),
        ("tn_cpu_temp",         True),
        ("tn_uptime",           True),
    ]),
    ("cat_rtc", [                               # 时钟（系统之后，存储之前）
        ("tn_rtc_device", True),
        ("tn_rtc_read",   True),
        ("tn_rtc_tick",   True),
        ("tn_rtc_write",  True),
    ]),
    ("cat_storage", [
        ("tn_ddr_capacity",  True),
        ("tn_ddr_bandwidth", True),
        ("tn_sd_capacity",   True),
        ("tn_sd_read",       True),
        ("tn_sd_write",      True),
    ]),
    ("cat_network", [
        ("tn_eth_speed",   True),
        ("tn_eth_iperf",   True),
        ("tn_bt_scan",     True),
        ("tn_bt_signal",   True),
        ("tn_wifi_scan",   True),
        ("tn_wifi_signal", True),
    ]),
    ("cat_usb", [
        ("tn_usb_hub",  True),
        ("tn_usb_enum", True),
        ("tn_usb_read", True),
    ]),
    ("cat_audio", [
        ("tn_hdmi_audio",         True),
        ("tn_headphone_loopback", True),    # 工厂有 3.5mm 环回治具，默认勾选
    ]),
    ("cat_expansion", [
        ("tn_gpio_loopback", True),        # 40-pin GPIO 回环
    ]),
    ("cat_display", [
        ("tn_hdmi_status",    True),
        ("tn_hdmi_screen",    True),
        ("tn_led_red",        True),
        ("tn_led_green",      True),
        ("tn_imx219_detect",  True),
        ("tn_imx219_capture", True),
    ]),
    ("cat_power", [
        ("tn_fan_control", True),
        ("tn_button",      True),         # 按键测试，需要人工操作
        ("tn_rtc_battery", True),         # 电池保持，需要人工断电/上电
    ]),
    # TODO: 后续逐类增量添加
    # ("cat_audio",     [...]),
    # ("cat_expansion", [...]),
]


# ── 网格背景（与其他页一致）──────────────────────────────────────────────────
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


# ── 测试选择页 ────────────────────────────────────────────────────────────────
class SelectPage(QWidget):
    back_requested  = Signal()
    start_requested = Signal(list)   # 选中的测试项 key 列表

    def __init__(self, app=None, parent: QWidget | None = None):
        super().__init__(parent)
        self._app = app
        self._build_ui()
        self._apply_lang()

    # ── UI 构建 ───────────────────────────────────────────────────────────────
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

    # ── 主内容 ────────────────────────────────────────────────────────────────
    def _make_content(self) -> QWidget:
        bg = _GridWidget()
        lay = QHBoxLayout(bg)
        lay.setContentsMargins(28, 24, 28, 24)
        lay.setSpacing(20)
        lay.addWidget(self._make_tree_panel(), stretch=6)
        lay.addWidget(self._make_summary_panel(), stretch=3)
        return bg

    # ── 左侧：测试树 ──────────────────────────────────────────────────────────
    def _make_tree_panel(self) -> QWidget:
        card = self._card()
        lay  = QVBoxLayout(card)
        lay.setContentsMargins(16, 14, 16, 14)
        lay.setSpacing(10)

        self._tree_title = QLabel()
        self._tree_title.setStyleSheet(
            f"color:{C_TEXT}; font-size:13px; font-weight:bold; background:transparent;"
        )
        lay.addWidget(self._tree_title)

        self._tree = QTreeWidget()
        self._tree.setColumnCount(1)
        self._tree.setHeaderHidden(True)
        self._tree.setStyleSheet(f"""
            QTreeWidget {{
                background:{C_BG}; border:1px solid {C_BORDER};
                border-radius:6px; color:{C_TEXT}; font-size:13px;
                outline:none;
            }}
            QTreeWidget::item {{
                padding:5px 4px; border-bottom:1px solid {C_BORDER};
            }}
            QTreeWidget::item:selected {{
                background:{C_ACCENT}22; color:{C_ACCENT};
            }}
            QTreeWidget::item:hover {{ background:{C_CARD2}; }}
            QTreeWidget::branch {{ background:{C_BG}; }}

            QTreeWidget::indicator {{
                width:15px; height:15px; border-radius:3px;
            }}
            QTreeWidget::indicator:unchecked {{
                border:1px solid #6e7681;
                background:{C_CARD2};
            }}
            QTreeWidget::indicator:checked {{
                border:1px solid #3a8fa8;
                background:#1a4a5c;
                image: url("{_CHECK_SVG}");
            }}
            QTreeWidget::indicator:indeterminate {{
                border:1px solid #3a8fa8;
                background:#122e38;
                image: url("{_DASH_SVG}");
            }}
            QTreeWidget::indicator:unchecked:hover {{
                border-color:#3a8fa8;
            }}
        """)
        self._tree.itemChanged.connect(self._on_item_changed)
        lay.addWidget(self._tree, stretch=1)
        return card

    # ── 右侧：汇总 ────────────────────────────────────────────────────────────
    def _make_summary_panel(self) -> QWidget:
        card = self._card()
        lay  = QVBoxLayout(card)
        lay.setContentsMargins(16, 14, 16, 14)
        lay.setSpacing(14)

        self._summary_title = QLabel()
        self._summary_title.setStyleSheet(
            f"color:{C_TEXT}; font-size:13px; font-weight:bold; background:transparent;"
        )
        lay.addWidget(self._summary_title)

        # 已选计数
        self._count_lbl = QLabel()
        self._count_lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._count_lbl.setStyleSheet(f"""
            color:{C_ACCENT}; font-size:32px; font-weight:bold;
            font-family:'Courier New','Consolas',monospace;
            background:transparent;
        """)
        lay.addWidget(self._count_lbl)

        self._count_sub = QLabel()
        self._count_sub.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._count_sub.setStyleSheet(
            f"color:{C_MUTED}; font-size:12px; background:transparent;"
        )
        lay.addWidget(self._count_sub)

        # 分隔线
        sep = QFrame()
        sep.setFrameShape(QFrame.Shape.HLine)
        sep.setStyleSheet(f"background:{C_BORDER};")
        sep.setFixedHeight(1)
        lay.addWidget(sep)

        # 全选 / 全不选
        btn_row = QHBoxLayout()
        btn_row.setSpacing(8)
        self._select_all_btn = self._action_btn("")
        self._select_all_btn.clicked.connect(self._on_select_all)
        btn_row.addWidget(self._select_all_btn)

        self._deselect_all_btn = self._action_btn("")
        self._deselect_all_btn.clicked.connect(self._on_deselect_all)
        btn_row.addWidget(self._deselect_all_btn)
        lay.addLayout(btn_row)

        lay.addStretch()
        return card

    # ── 底栏 ─────────────────────────────────────────────────────────────────
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

        self._continue_btn = self._nav_btn("", primary=True)
        self._continue_btn.clicked.connect(self._on_continue)
        lay.addWidget(self._continue_btn)
        return bar

    # ── 工具 ─────────────────────────────────────────────────────────────────
    def _card(self) -> QWidget:
        w = QWidget()
        w.setStyleSheet(
            f"background:{C_CARD}; border-radius:10px; border:1px solid {C_BORDER};"
        )
        return w

    def _action_btn(self, text: str) -> QPushButton:
        btn = QPushButton(text)
        btn.setFixedHeight(34)
        btn.setCursor(Qt.CursorShape.PointingHandCursor)
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
                QPushButton:hover   {{ background:{C_ACCENT}; }}
                QPushButton:pressed {{ background:#0284c7; }}
                QPushButton:disabled {{ background:#21262d; color:{C_MUTED}; }}
            """)
        else:
            btn.setStyleSheet(f"""
                QPushButton {{
                    color:{C_TEXT}; background:transparent;
                    border:1px solid {C_BORDER}; border-radius:6px;
                    font-size:13px;
                }}
                QPushButton:hover   {{ border-color:{C_ACCENT}; color:{C_ACCENT}; }}
                QPushButton:pressed {{ background:{C_CARD2}; }}
            """)
        return btn

    # ── 构建测试树 ────────────────────────────────────────────────────────────
    def _build_tree(self):
        self._tree.blockSignals(True)
        self._tree.clear()

        for cat_key, items in _TEST_TREE:
            cat_item = QTreeWidgetItem(self._tree)
            cat_item.setText(0, t(cat_key))
            cat_item.setFlags(
                cat_item.flags()
                | Qt.ItemFlag.ItemIsUserCheckable
                | Qt.ItemFlag.ItemIsAutoTristate
            )
            cat_item.setCheckState(0, Qt.CheckState.Checked)

            all_checked = True
            for entry in items:
                # 兼容 (key, default) 和 (key, default, disabled) 两种格式
                name_key  = entry[0]
                default   = entry[1]
                disabled  = entry[2] if len(entry) > 2 else False

                child = QTreeWidgetItem(cat_item)
                child.setText(0, t(name_key))
                child.setData(0, Qt.ItemDataRole.UserRole,     name_key)
                child.setData(0, Qt.ItemDataRole.UserRole + 1, disabled)

                if disabled:
                    # 不可勾选 + 灰色文字 + 提示
                    flags = child.flags()
                    flags &= ~Qt.ItemFlag.ItemIsUserCheckable
                    flags &= ~Qt.ItemFlag.ItemIsEnabled
                    child.setFlags(flags)
                    child.setForeground(0, QColor("#4a5560"))
                    child.setCheckState(0, Qt.CheckState.Unchecked)
                    child.setToolTip(0, "等待测试工装支持")
                    all_checked = False
                else:
                    child.setFlags(child.flags() | Qt.ItemFlag.ItemIsUserCheckable)
                    state = Qt.CheckState.Checked if default else Qt.CheckState.Unchecked
                    child.setCheckState(0, state)
                    if not default:
                        all_checked = False

            if not all_checked:
                cat_item.setCheckState(0, Qt.CheckState.PartiallyChecked)

            cat_item.setExpanded(True)

        self._tree.blockSignals(False)
        self._refresh_count()

    # ── 事件 ─────────────────────────────────────────────────────────────────
    def _on_item_changed(self, item: QTreeWidgetItem, col: int):
        self._refresh_count()

    def _is_disabled(self, item: QTreeWidgetItem) -> bool:
        return bool(item.data(0, Qt.ItemDataRole.UserRole + 1))

    def _on_select_all(self):
        self._tree.blockSignals(True)
        for i in range(self._tree.topLevelItemCount()):
            cat = self._tree.topLevelItem(i)
            for j in range(cat.childCount()):
                child = cat.child(j)
                if self._is_disabled(child):
                    continue
                child.setCheckState(0, Qt.CheckState.Checked)
        self._tree.blockSignals(False)
        self._refresh_count()

    def _on_deselect_all(self):
        self._tree.blockSignals(True)
        for i in range(self._tree.topLevelItemCount()):
            cat = self._tree.topLevelItem(i)
            for j in range(cat.childCount()):
                child = cat.child(j)
                if self._is_disabled(child):
                    continue
                child.setCheckState(0, Qt.CheckState.Unchecked)
        self._tree.blockSignals(False)
        self._refresh_count()

    def _refresh_count(self):
        selected, total = 0, 0
        for i in range(self._tree.topLevelItemCount()):
            cat = self._tree.topLevelItem(i)
            for j in range(cat.childCount()):
                child = cat.child(j)
                if self._is_disabled(child):
                    continue
                total += 1
                if child.checkState(0) == Qt.CheckState.Checked:
                    selected += 1
        self._count_lbl.setText(f"{selected}")
        self._count_sub.setText(t("sel_count_sub", selected, total))
        self._continue_btn.setEnabled(selected > 0)

    def _selected_keys(self) -> list[str]:
        keys = []
        for i in range(self._tree.topLevelItemCount()):
            cat = self._tree.topLevelItem(i)
            for j in range(cat.childCount()):
                child = cat.child(j)
                if self._is_disabled(child):
                    continue
                if child.checkState(0) == Qt.CheckState.Checked:
                    key = child.data(0, Qt.ItemDataRole.UserRole)
                    if key:
                        keys.append(key)
        return keys

    def _on_continue(self):
        self.start_requested.emit(self._selected_keys())

    # ── 语言 ─────────────────────────────────────────────────────────────────
    def apply_lang(self):
        self._apply_lang()

    def _apply_lang(self):
        self._tree_title.setText(t("sel_title"))
        self._summary_title.setText(t("sel_summary_title"))
        self._select_all_btn.setText(t("sel_select_all"))
        self._deselect_all_btn.setText(t("sel_deselect_all"))
        self._back_btn.setText(t("btn_back"))
        self._continue_btn.setText(t("btn_continue"))
        self._build_tree()   # 重建树以刷新语言

    def on_show(self):
        pass
