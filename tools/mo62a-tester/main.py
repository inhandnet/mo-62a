#!/usr/bin/env python3
"""MO-62A 自动化测试工具"""
import sys
from gui.app import App


def main():
    app = App()
    app.mainloop()


if __name__ == "__main__":
    main()
