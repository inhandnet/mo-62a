"""运行页 — 测试执行、实时结果展示、报告保存"""

from __future__ import annotations

from datetime import datetime
from pathlib import Path

from PySide6.QtCore import Qt, Signal, QThread, QTimer
from PySide6.QtGui import QPainter, QColor, QPen
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel, QPushButton,
    QProgressBar, QTableWidget, QTableWidgetItem, QHeaderView,
    QMessageBox, QFileDialog, QSizePolicy,
)

from config.i18n import t
from config.settings import APP_VERSION, REPORT_DIR
from interface.base import Status

# ── 颜色 ─────────────────────────────────────────────────────────────────────
C_BG      = "#0d1117"
C_BAR     = "#161b22"
C_BORDER  = "#21262d"
C_ACCENT  = "#00d4ff"
C_ACCENT2 = "#0ea5e9"
C_TEXT    = "#e6edf3"
C_MUTED   = "#8b949e"
C_CARD    = "#161b22"
C_CARD2   = "#1c2333"

STATUS_COLOR = {
    Status.INFO: "#00d4ff",
    Status.PASS: "#3fb950",
    Status.FAIL: "#f85149",
    Status.SKIP: "#8b949e",
}

# ── 网格背景 ──────────────────────────────────────────────────────────────────
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


# ── 后台测试执行线程 ──────────────────────────────────────────────────────────
class _TestRunner(QThread):
    test_started  = Signal(int)                        # row index
    test_finished = Signal(int, str, str, float, list) # row, status, message, duration, images
    all_finished  = Signal()

    def __init__(self, tests: list):
        super().__init__()
        self._tests   = tests
        self._running = True

    def stop(self):
        self._running = False

    def run(self):
        for i, test in enumerate(self._tests):
            if not self._running:
                break
            self.test_started.emit(i)
            result = test.run()
            self.test_finished.emit(
                i, result.status, result.message, result.duration,
                getattr(result, "images", []) or [],
            )
        self.all_finished.emit()


# ── 运行页 ────────────────────────────────────────────────────────────────────
class RunPage(QWidget):
    back_requested = Signal()

    def __init__(self, app=None, parent: QWidget | None = None):
        super().__init__(parent)
        self._app      = app
        self._tests    = []
        self._runner   = None
        self._running  = False
        self._reporter = None

        # 动画 spinner
        self._spinner_frames = ["⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"]
        self._spinner_idx    = 0
        self._spinner_row    = -1
        self._spinner_timer  = QTimer(self)
        self._spinner_timer.setInterval(100)
        self._spinner_timer.timeout.connect(self._tick_spinner)

        self._build_ui()
        self._apply_lang()

    # ── UI 构建 ───────────────────────────────────────────────────────────────
    def _build_ui(self):
        root = QVBoxLayout(self)
        root.setContentsMargins(0, 0, 0, 0)
        root.setSpacing(0)
        root.addWidget(self._make_topbar())
        root.addWidget(self._make_progress_bar())
        root.addWidget(self._make_table_area(), stretch=1)
        root.addWidget(self._make_summary_bar())
        root.addWidget(self._make_footer())

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

    def _make_progress_bar(self) -> QWidget:
        bar = QWidget()
        bar.setFixedHeight(70)
        bar.setStyleSheet(f"background:{C_BAR}; border-bottom:1px solid {C_BORDER};")
        lay = QVBoxLayout(bar)
        lay.setContentsMargins(28, 10, 28, 10)
        lay.setSpacing(6)

        # 进度条 + 计数
        top_row = QHBoxLayout()
        self._progress = QProgressBar()
        self._progress.setFixedHeight(8)
        self._progress.setTextVisible(False)
        self._progress.setStyleSheet(f"""
            QProgressBar {{
                background:{C_CARD2}; border-radius:4px; border:none;
            }}
            QProgressBar::chunk {{
                background:qlineargradient(x1:0,y1:0,x2:1,y2:0,
                    stop:0 {C_ACCENT2}, stop:1 {C_ACCENT});
                border-radius:4px;
            }}
        """)
        top_row.addWidget(self._progress)

        self._count_lbl = QLabel("0 / 0")
        self._count_lbl.setFixedWidth(60)
        self._count_lbl.setAlignment(Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignVCenter)
        self._count_lbl.setStyleSheet(f"color:{C_MUTED}; font-size:12px; background:transparent;")
        top_row.addWidget(self._count_lbl)
        lay.addLayout(top_row)

        # 当前测试名
        self._current_lbl = QLabel("")
        self._current_lbl.setStyleSheet(f"color:{C_MUTED}; font-size:12px; background:transparent;")
        lay.addWidget(self._current_lbl)
        return bar

    def _make_table_area(self) -> QWidget:
        bg = _GridWidget()
        lay = QVBoxLayout(bg)
        lay.setContentsMargins(28, 20, 28, 20)

        self._table = QTableWidget()
        self._table.setColumnCount(4)
        self._table.setEditTriggers(QTableWidget.EditTrigger.NoEditTriggers)
        self._table.setSelectionMode(QTableWidget.SelectionMode.SingleSelection)
        self._table.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self._table.verticalHeader().setVisible(False)
        self._table.horizontalHeader().setStretchLastSection(False)
        self._table.horizontalHeader().setSectionResizeMode(0, QHeaderView.ResizeMode.Fixed)
        self._table.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeMode.Fixed)
        self._table.horizontalHeader().setSectionResizeMode(2, QHeaderView.ResizeMode.Stretch)
        self._table.horizontalHeader().setSectionResizeMode(3, QHeaderView.ResizeMode.Fixed)
        self._table.setColumnWidth(0, 160)
        self._table.setColumnWidth(1, 80)
        self._table.setColumnWidth(3, 80)
        self._table.setStyleSheet(f"""
            QTableWidget {{
                background:{C_CARD}; border:1px solid {C_BORDER};
                border-radius:8px; color:{C_TEXT}; font-size:13px;
                gridline-color:{C_BORDER}; outline:none;
            }}
            QTableWidget::item {{ padding:8px 10px; }}
            QTableWidget::item:selected {{ background:{C_ACCENT}22; color:{C_TEXT}; }}
            QHeaderView::section {{
                background:{C_BG}; color:{C_MUTED}; font-size:12px;
                padding:8px 10px; border:none;
                border-bottom:1px solid {C_BORDER};
            }}
        """)
        lay.addWidget(self._table)
        return bg

    def _make_summary_bar(self) -> QWidget:
        self._summary_bar = QWidget()
        self._summary_bar.setFixedHeight(36)
        self._summary_bar.setStyleSheet(
            f"background:{C_BAR}; border-top:1px solid {C_BORDER};"
        )
        lay = QHBoxLayout(self._summary_bar)
        lay.setContentsMargins(28, 0, 28, 0)
        self._summary_lbl = QLabel("")
        self._summary_lbl.setStyleSheet(
            f"color:{C_MUTED}; font-size:12px; background:transparent;"
        )
        lay.addWidget(self._summary_lbl)
        lay.addStretch()
        self._summary_bar.hide()
        return self._summary_bar

    def _make_footer(self) -> QWidget:
        bar = QWidget()
        bar.setFixedHeight(52)
        bar.setStyleSheet(f"background:{C_BAR}; border-top:1px solid {C_BORDER};")
        lay = QHBoxLayout(bar)
        lay.setContentsMargins(24, 0, 24, 0)
        lay.setSpacing(10)

        self._back_btn = self._nav_btn("", primary=False)
        self._back_btn.clicked.connect(self._on_back)
        lay.addWidget(self._back_btn)

        lay.addStretch()

        self._rerun_btn = self._nav_btn("", primary=False)
        self._rerun_btn.setEnabled(False)
        self._rerun_btn.clicked.connect(self._on_rerun)
        lay.addWidget(self._rerun_btn)

        self._report_btn = self._nav_btn("", primary=True)
        self._report_btn.setEnabled(False)
        self._report_btn.clicked.connect(self._on_save_report)
        lay.addWidget(self._report_btn)

        return bar

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
                QPushButton:disabled {{ color:{C_MUTED}; border-color:#30363d; }}
            """)
        return btn

    # ── 测试启动 ──────────────────────────────────────────────────────────────
    def start_tests(self, tests: list):
        """由 app 调用，传入测试实例列表后立即开始执行。"""
        from reporter.reporter import Reporter
        self._tests    = tests
        self._running  = True
        self._reporter = Reporter(self._app.device_info if self._app else {})

        self._rerun_btn.setEnabled(False)
        self._report_btn.setEnabled(False)
        self._summary_bar.hide()
        self._summary_lbl.setText("")

        # 初始化进度
        total = len(tests)
        self._progress.setMaximum(total)
        self._progress.setValue(0)
        self._count_lbl.setText(f"0 / {total}")
        self._current_lbl.setText("")

        # 预填表格（全部显示"等待"）
        self._table.setRowCount(total)
        for i, test in enumerate(tests):
            self._set_row(i, test.name, "", t("run_waiting"), "")

        # 启动后台线程
        self._runner = _TestRunner(tests)
        self._runner.test_started.connect(self._on_test_started)
        self._runner.test_finished.connect(self._on_test_finished)
        self._runner.all_finished.connect(self._on_all_finished)
        self._runner.start()

    # ── 事件处理 ──────────────────────────────────────────────────────────────
    def _on_test_started(self, row: int):
        name = self._tests[row].name
        self._current_lbl.setText(t("run_running", name))
        self._spinner_row = row
        self._spinner_idx = 0
        self._set_row(row, name, C_MUTED, self._spinner_frames[0], "")
        self._spinner_timer.start()

    def _on_test_finished(self, row: int, status: str, message: str,
                          duration: float, images: list = None):
        images = images or []
        self._spinner_timer.stop()
        self._spinner_row = -1
        name  = self._tests[row].name
        color = STATUS_COLOR.get(status, C_TEXT)
        dur   = f"{duration:.2f}s"
        self._set_row(row, name, color, status, message, dur)

        done = row + 1
        self._progress.setValue(done)
        self._count_lbl.setText(f"{done} / {len(self._tests)}")

        if self._reporter:
            self._reporter.add(name, status, message, duration, images)

    def _tick_spinner(self):
        if self._spinner_row < 0:
            return
        self._spinner_idx = (self._spinner_idx + 1) % len(self._spinner_frames)
        item = self._table.item(self._spinner_row, 1)
        if item:
            item.setText(self._spinner_frames[self._spinner_idx])

    def _on_all_finished(self):
        self._spinner_timer.stop()
        self._spinner_row = -1
        self._running = False
        self._current_lbl.setText("")
        self._rerun_btn.setEnabled(True)
        self._report_btn.setEnabled(True)

        # 汇总
        cnt = {s: 0 for s in (Status.INFO, Status.PASS, Status.FAIL, Status.SKIP)}
        for r in range(self._table.rowCount()):
            status = self._table.item(r, 1).text() if self._table.item(r, 1) else ""
            cnt[status] = cnt.get(status, 0) + 1

        self._summary_lbl.setText(
            t("run_summary", cnt[Status.INFO], cnt[Status.PASS],
              cnt[Status.FAIL], cnt[Status.SKIP])
        )
        self._summary_bar.show()

    def _on_back(self):
        if self._running:
            reply = QMessageBox.question(
                self, t("run_leave_title"), t("run_leave_msg"),
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No,
                QMessageBox.StandardButton.No,
            )
            if reply != QMessageBox.StandardButton.Yes:
                return
            if self._runner:
                self._runner.stop()
                self._runner.wait(2000)
        self.back_requested.emit()

    def _on_rerun(self):
        self.start_tests(self._tests)

    def _on_save_report(self):
        if not self._reporter:
            return
        ts       = datetime.now().strftime("%Y%m%d_%H%M%S")
        default  = Path(REPORT_DIR) / f"report_{ts}.html"
        filepath, _ = QFileDialog.getSaveFileName(
            self, t("run_btn_report"),
            str(default), "HTML (*.html)"
        )
        if not filepath:
            return
        try:
            saved = self._reporter.save(filepath)
            QMessageBox.information(self, t("run_btn_report"),
                                    f"{t('run_report_saved')}\n{saved}")
        except Exception as e:
            QMessageBox.warning(self, t("run_btn_report"),
                                t("run_report_fail", e))

    # ── 表格辅助 ──────────────────────────────────────────────────────────────
    def _set_row(self, row: int, name: str, color: str,
                 status: str, message: str, duration: str = ""):
        def _item(text, align=Qt.AlignmentFlag.AlignLeft | Qt.AlignmentFlag.AlignVCenter):
            item = QTableWidgetItem(text)
            item.setTextAlignment(align)
            return item

        center = Qt.AlignmentFlag.AlignCenter

        self._table.setItem(row, 0, _item(name))

        status_item = _item(status, center)
        if color:
            status_item.setForeground(QColor(color))
        self._table.setItem(row, 1, status_item)

        self._table.setItem(row, 2, _item(message))
        self._table.setItem(row, 3, _item(duration, center))

    # ── 语言 ─────────────────────────────────────────────────────────────────
    def apply_lang(self):
        self._apply_lang()

    def _apply_lang(self):
        headers = [t("run_col_name"), t("run_col_status"),
                   t("run_col_message"), t("run_col_duration")]
        self._table.setHorizontalHeaderLabels(headers)
        self._back_btn.setText(t("btn_back"))
        self._rerun_btn.setText(t("run_btn_rerun"))
        self._report_btn.setText(t("run_btn_report"))

    def on_show(self):
        pass
