"""全局常量配置"""

from pathlib import Path

# ── 应用信息 ──────────────────────────────────────────────────────────────────
APP_NAME    = "Mo 62A Auto Test"
APP_VERSION = "2.0.0"

# ── 路径 ──────────────────────────────────────────────────────────────────────
ROOT_DIR    = Path(__file__).parent.parent
PICTURE_DIR = ROOT_DIR / "picture"
REPORT_DIR  = ROOT_DIR / "reports"

# ── SSH 默认值 ────────────────────────────────────────────────────────────────
DEFAULT_SSH_USER = "debian"
DEFAULT_SSH_PORT = 22
DEFAULT_PASSWORD = "123456"

# ── 窗口默认尺寸 ──────────────────────────────────────────────────────────────
WIN_MIN_W = 900
WIN_MIN_H = 620
WIN_DEF_W = 1100
WIN_DEF_H = 720
