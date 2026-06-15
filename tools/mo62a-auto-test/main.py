#!/usr/bin/env python3
"""
MO 62A 自动化测试工具
主入口 — 初始化 QApplication 并启动主窗口。
"""

import sys
from PySide6.QtWidgets import QApplication
from PySide6.QtCore import Qt

from config.settings import APP_NAME, APP_VERSION


def main() -> None:
    QApplication.setHighDpiScaleFactorRoundingPolicy(
        Qt.HighDpiScaleFactorRoundingPolicy.PassThrough
    )

    app = QApplication(sys.argv)
    app.setApplicationName(APP_NAME)
    app.setApplicationVersion(APP_VERSION)

    from gui.app import MainWindow
    window = MainWindow()
    window.show()

    sys.exit(app.exec())


if __name__ == "__main__":
    main()
