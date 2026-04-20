"""
全局字体配置。

使用等宽字体（Courier New）确保所有英文字母和中文字符占据相同宽度。
Windows 上中文字符会自动回落到 NSimSun（新宋体），同样是等宽字体。
"""
import platform
import tkinter.font as tkfont


def _mono_family() -> str:
    """根据平台选择最佳等宽字体。"""
    sys = platform.system()
    if sys == "Windows":
        return "Courier New"
    elif sys == "Darwin":
        return "Menlo"
    else:
        return "DejaVu Sans Mono"


MONO = _mono_family()

# ── 字体元组常量（供所有 widget 的 font= 参数使用） ──────────────────────
F_NORMAL = (MONO, 10)
F_BOLD   = (MONO, 10, "bold")
F_SMALL  = (MONO, 9)
F_TITLE  = (MONO, 14, "bold")
F_HEADER = (MONO, 13, "bold")
F_CODE   = (MONO, 9)       # 日志区等代码字体


def apply_global(root) -> None:
    """
    覆盖 tkinter 全局命名字体，使未显式指定 font= 的控件
    （如 ttk.Treeview、ttk.Scrollbar、默认 Label 等）也继承等宽字体。
    必须在主窗口创建后、页面构建前调用。
    """
    for name in ("TkDefaultFont", "TkTextFont", "TkMenuFont",
                 "TkHeadingFont", "TkCaptionFont", "TkSmallCaptionFont",
                 "TkIconFont", "TkTooltipFont"):
        try:
            f = tkfont.nametofont(name)
            f.configure(family=MONO, size=10)
        except Exception:
            pass

    # TkFixedFont 单独设置，避免被覆盖成非等宽
    try:
        tkfont.nametofont("TkFixedFont").configure(family=MONO, size=9)
    except Exception:
        pass
