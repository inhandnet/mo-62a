"""欢迎页 — 初始界面"""

from __future__ import annotations
import sys
import platform
from pathlib import Path

import PySide6
from PySide6.QtCore import Qt, Signal
from PySide6.QtGui import QPixmap, QPainter, QColor, QPen, QLinearGradient
from PySide6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QLabel,
    QPushButton, QGraphicsDropShadowEffect, QSizePolicy,
)

from config.settings import PICTURE_DIR, APP_VERSION
from config.i18n import t, set_lang, get_lang

# ── 颜色常量 ──────────────────────────────────────────────────────────────────
C_BG      = "#0d1117"
C_BAR     = "#161b22"
C_BORDER  = "#21262d"
C_ACCENT  = "#00d4ff"
C_ACCENT2 = "#0ea5e9"
C_TEXT    = "#e6edf3"
C_MUTED   = "#8b949e"
C_CARD    = "#161b22"

PRODUCT_IMAGE = PICTURE_DIR / "62A-03.png"


# ── 带点阵网格的背景 Widget ───────────────────────────────────────────────────
class _GridWidget(QWidget):
    """深色背景 + 细点阵网格，纯 QPainter 绘制。"""

    def paintEvent(self, event):
        p = QPainter(self)
        p.fillRect(self.rect(), QColor(C_BG))

        # 网格线
        pen = QPen(QColor("#161b22"))
        pen.setWidth(1)
        p.setPen(pen)
        step = 40
        for x in range(0, self.width() + step, step):
            p.drawLine(x, 0, x, self.height())
        for y in range(0, self.height() + step, step):
            p.drawLine(0, y, self.width(), y)

        # 交叉点高亮小圆
        p.setPen(Qt.PenStyle.NoPen)
        p.setBrush(QColor("#1c2333"))
        for x in range(0, self.width() + step, step):
            for y in range(0, self.height() + step, step):
                p.drawEllipse(x - 1, y - 1, 3, 3)

        p.end()


# ── 欢迎页 ────────────────────────────────────────────────────────────────────
class WelcomePage(QWidget):
    """初始欢迎界面，包含语言切换、产品图片和继续按钮。"""

    continue_requested = Signal()

    def __init__(self, app=None, parent: QWidget | None = None):
        super().__init__(parent)
        self._app = app
        self._build_ui()
        self._apply_lang()

    # ── 构建 UI ──────────────────────────────────────────────────────────────
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
        bar.setStyleSheet(
            f"background-color: {C_BAR};"
            f"border-bottom: 1px solid {C_BORDER};"
        )
        layout = QHBoxLayout(bar)
        layout.setContentsMargins(24, 0, 24, 0)

        brand = QLabel("◈  Mo 62A Auto Test")
        brand.setStyleSheet(
            f"color: {C_ACCENT}; font-size: 14px; font-weight: bold;"
            "font-family: 'Courier New', 'Consolas', monospace;"
            "background: transparent; letter-spacing: 1px;"
        )
        layout.addWidget(brand)
        layout.addStretch()

        # 语言切换：单按钮，显示对立语言，点击切换
        self._lang_toggle_btn = QPushButton()
        self._lang_toggle_btn.setFixedSize(52, 28)
        self._lang_toggle_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self._lang_toggle_btn.setStyleSheet(f"""
            QPushButton {{
                color: {C_ACCENT};
                background: transparent;
                border: 1px solid {C_ACCENT};
                border-radius: 5px;
                font-size: 12px;
                font-weight: bold;
            }}
            QPushButton:hover {{
                background: {C_ACCENT}22;
            }}
        """)
        self._lang_toggle_btn.clicked.connect(self._toggle_lang)
        layout.addWidget(self._lang_toggle_btn)

        return bar

    # ── 主内容区 ─────────────────────────────────────────────────────────────
    def _make_content(self) -> QWidget:
        bg = _GridWidget()

        layout = QVBoxLayout(bg)
        layout.setContentsMargins(40, 36, 40, 36)
        layout.setSpacing(0)
        layout.setAlignment(Qt.AlignmentFlag.AlignCenter)

        # 产品图片卡片
        layout.addWidget(self._make_image_card())
        layout.addSpacing(28)

        # 主标题
        self._title_lbl = QLabel()
        self._title_lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._title_lbl.setStyleSheet(f"""
            color: {C_TEXT};
            font-size: 28px;
            font-weight: bold;
            font-family: 'Courier New', 'Consolas', monospace;
            background: transparent;
            letter-spacing: 3px;
        """)
        layout.addWidget(self._title_lbl)
        layout.addSpacing(10)

        # 副标题
        self._subtitle_lbl = QLabel()
        self._subtitle_lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self._subtitle_lbl.setStyleSheet(f"""
            color: {C_MUTED};
            font-size: 13px;
            background: transparent;
            letter-spacing: 1px;
        """)
        layout.addWidget(self._subtitle_lbl)
        layout.addSpacing(36)

        # 继续按钮
        self._continue_btn = QPushButton()
        self._continue_btn.setFixedSize(180, 46)
        self._continue_btn.setCursor(Qt.CursorShape.PointingHandCursor)
        self._continue_btn.setStyleSheet(f"""
            QPushButton {{
                background: qlineargradient(
                    x1:0, y1:0, x2:1, y2:0,
                    stop:0 {C_ACCENT2}, stop:1 {C_ACCENT}
                );
                color: #0d1117;
                border: none;
                border-radius: 6px;
                font-size: 14px;
                font-weight: bold;
                letter-spacing: 2px;
            }}
            QPushButton:hover {{
                background: qlineargradient(
                    x1:0, y1:0, x2:1, y2:0,
                    stop:0 #38bdf8, stop:1 #22d3ee
                );
            }}
            QPushButton:pressed {{
                background: #0284c7;
            }}
        """)
        # 按钮发光效果
        glow = QGraphicsDropShadowEffect()
        glow.setBlurRadius(20)
        glow.setColor(QColor(0, 212, 255, 120))
        glow.setOffset(0, 0)
        self._continue_btn.setGraphicsEffect(glow)
        self._continue_btn.clicked.connect(self.continue_requested)

        btn_row = QHBoxLayout()
        btn_row.setAlignment(Qt.AlignmentFlag.AlignCenter)
        btn_row.addWidget(self._continue_btn)
        layout.addLayout(btn_row)

        return bg

    def _make_image_card(self) -> QWidget:
        """产品图片带深色圆角卡片和阴影。"""
        card = QWidget()
        card.setStyleSheet(f"""
            background-color: {C_CARD};
            border-radius: 12px;
            border: 1px solid #21262d;
        """)
        card.setSizePolicy(QSizePolicy.Policy.Fixed, QSizePolicy.Policy.Fixed)

        shadow = QGraphicsDropShadowEffect()
        shadow.setBlurRadius(40)
        shadow.setColor(QColor(0, 212, 255, 60))
        shadow.setOffset(0, 8)
        card.setGraphicsEffect(shadow)

        card_layout = QVBoxLayout(card)
        card_layout.setContentsMargins(20, 20, 20, 20)

        img_lbl = QLabel()
        img_lbl.setAlignment(Qt.AlignmentFlag.AlignCenter)
        img_lbl.setStyleSheet("background: transparent;")

        if PRODUCT_IMAGE.exists():
            pix = QPixmap(str(PRODUCT_IMAGE))
            pix = pix.scaledToWidth(420, Qt.TransformationMode.SmoothTransformation)
            img_lbl.setPixmap(pix)
        else:
            img_lbl.setText("[ Mo 62A ]")
            img_lbl.setFixedSize(420, 260)
            img_lbl.setStyleSheet(f"color: {C_MUTED}; font-size: 20px; background: transparent;")

        card_layout.addWidget(img_lbl)
        return card

    # ── 底栏 ─────────────────────────────────────────────────────────────────
    def _make_footer(self) -> QWidget:
        bar = QWidget()
        bar.setFixedHeight(36)
        bar.setStyleSheet(
            f"background-color: {C_BAR};"
            f"border-top: 1px solid {C_BORDER};"
        )
        layout = QHBoxLayout(bar)
        layout.setContentsMargins(24, 0, 24, 0)

        py_ver   = f"Python {sys.version_info.major}.{sys.version_info.minor}"
        ps6_ver  = f"PySide6 {PySide6.__version__}"
        os_info  = f"{platform.system()} {platform.machine()}"
        info_txt = f"v{APP_VERSION}  ·  {py_ver}  ·  {ps6_ver}  ·  {os_info}"

        lbl = QLabel(info_txt)
        lbl.setStyleSheet(
            f"color: {C_MUTED}; font-size: 11px; background: transparent;"
        )
        layout.addWidget(lbl)
        layout.addStretch()

        return bar

    # ── 语言切换 ─────────────────────────────────────────────────────────────
    def _toggle_lang(self):
        new_lang = "en" if get_lang() == "zh" else "zh"
        if self._app:
            self._app.apply_lang_all(new_lang)
        else:
            set_lang(new_lang)
            self._apply_lang()

    def apply_lang(self):
        """供 MainWindow.apply_lang_all() 调用。"""
        self._apply_lang()

    def _apply_lang(self):
        # 按钮显示对立语言（当前中文则显示 EN，反之亦然）
        self._lang_toggle_btn.setText("EN" if get_lang() == "zh" else "中文")
        self._title_lbl.setText(t("welcome_title"))
        self._subtitle_lbl.setText(t("welcome_subtitle"))
        self._continue_btn.setText(t("welcome_continue"))
