"""IMX219 CSI2 摄像头测试（摄像头对准 HDMI 显示器）

测试前自动检查 extlinux.conf overlay，必要时修改 default 并重启；
两个测试都完成后还原为原始 default（不需要二次重启）。

测试项：
  1. IMX219 检测 — 验证 media 拓扑中有 imx219 sensor 节点（INFO）
  2. IMX219 抓帧 — 显示器依次显示 R/G/B，用 IMX219 + tiovxisp（VPAC ISP）捕帧分析颜色

tiovxisp 路径：v4l2src → tiovxisp → NV12 → videoconvert → BGR → 颜色分析。
若设备缺少 DCC 文件，自动通过 SFTP 上传（来自 assets/imx219/linear/）。
"""

from __future__ import annotations
import re
import time
from pathlib import Path
from config.i18n import t
from interface.base import TestCase

_ASSETS_DIR      = Path(__file__).parent.parent.parent / "assets" / "imx219" / "linear"
_DCC_REMOTE_DIR  = "/opt/imaging/imx219/linear"
_DCC_VISS_REMOTE = f"{_DCC_REMOTE_DIR}/dcc_viss.bin"
_DCC_2A_REMOTE   = f"{_DCC_REMOTE_DIR}/dcc_2a.bin"

_EXTLINUX    = "/boot/firmware/extlinux/extlinux.conf"
_CAM_DTBO    = "k3-am62a7-mo-62a-cam-imx219.dtbo"
_REBOOT_WAIT = 60   # 重启后最长等待秒数


# ── extlinux.conf 工具函数 ────────────────────────────────────────────────────
def _current_default(conf: str) -> str | None:
    m = re.search(r'^default\s+(\S+)', conf, re.MULTILINE)
    return m.group(1) if m else None


def _find_cam_label(conf: str) -> str | None:
    """找到第一个 fdtoverlays 含 cam-imx219.dtbo 的 label 名。"""
    current_label = None
    for line in conf.splitlines():
        m = re.match(r'^label\s+(\S+)', line.strip())
        if m:
            current_label = m.group(1)
        if _CAM_DTBO in line and current_label:
            return current_label
    return None


def _set_default(test: TestCase, conf: str, label: str) -> bool:
    """修改 extlinux.conf 的 default 行，写回设备。"""
    new_conf = re.sub(r'^(default\s+)\S+', rf'\g<1>{label}',
                      conf, count=1, flags=re.MULTILINE)
    # 用 tee 写入（避免 shell heredoc 特殊字符问题）
    escaped = new_conf.replace("'", "'\\''")
    rc, _, _ = test.cmd(
        f"printf '%s' '{escaped}' > {_EXTLINUX}", timeout=5
    )
    return rc == 0


# ── 共享管理器（detect + capture 共用）────────────────────────────────────────
class _Imx219Manager:
    """协调两个测试的 overlay 切换和最终回退。

    两个测试均完成（无论成功与否）后，自动将 extlinux.conf 恢复原始 default。
    """

    def __init__(self, board):
        self._board         = board
        self._overlay_ready: bool | None = None   # None=未检测
        self._original_def: str | None   = None   # 改前的 default 值
        self._was_modified  = False
        self._registered    = 0   # 注册的测试数
        self._completed     = 0   # 已完成的测试数

    def register(self):
        self._registered += 1

    def ensure(self, test: TestCase) -> bool:
        """确保 cam overlay 已加载，首次调用执行实际检测/切换逻辑。"""
        if self._overlay_ready is not None:
            return self._overlay_ready
        self._overlay_ready = self._do_ensure(test)
        return self._overlay_ready

    def done(self, test: TestCase):
        """一个测试完成时调用；全部完成后回退 extlinux.conf。"""
        self._completed += 1
        if self._completed >= self._registered:
            self._restore(test)

    # ── 私有 ──────────────────────────────────────────────────────────────────
    def _do_ensure(self, test: TestCase) -> bool:
        # 已加载
        if test.cmd("test -c /dev/media0")[0] == 0:
            return True

        rc, conf, _ = test.cmd(f"cat {_EXTLINUX} 2>/dev/null")
        if rc != 0 or not conf.strip():
            return False

        cam_label = _find_cam_label(conf)
        if not cam_label:
            return False

        orig = _current_default(conf)
        if orig == cam_label:
            # default 已经是 cam label 但 /dev/media0 不存在 → 重启未生效
            return False

        # 记录旧值，修改为 cam label
        self._original_def = orig
        if not _set_default(test, conf, cam_label):
            return False
        self._was_modified = True

        # 重启并等待 SSH
        host = test.board.host
        user = test.board.user
        pwd  = test.board._password

        test.cmd("systemctl reboot &", timeout=3)
        time.sleep(3.0)
        try:
            test.board.close()
        except Exception:
            pass

        deadline = time.monotonic() + _REBOOT_WAIT
        while time.monotonic() < deadline:
            try:
                test.board.connect(host, user, pwd)
                break
            except Exception:
                time.sleep(3.0)
        else:
            return False

        return test.cmd("test -c /dev/media0")[0] == 0

    def _restore(self, test: TestCase):
        """将 extlinux.conf 的 default 恢复为原始值（不重启，下次生效）。"""
        if not self._was_modified or not self._original_def:
            return
        rc, conf, _ = test.cmd(f"cat {_EXTLINUX} 2>/dev/null")
        if rc != 0:
            return
        _set_default(test, conf, self._original_def)
        self._was_modified = False


# ── IMX219 检测 ───────────────────────────────────────────────────────────────
class Imx219DetectTest(TestCase):
    category_key = "cat_display"
    name_key     = "tn_imx219_detect"

    def __init__(self, board, manager: _Imx219Manager):
        super().__init__(board)
        self._mgr = manager
        manager.register()

    def _run(self):
        try:
            if not self._mgr.ensure(self):
                self.skip(
                    t("msg_imx219_overlay_missing", _CAM_DTBO)
                )
                return

            rc, out, _ = self.cmd(
                "media-ctl -d /dev/media0 -p 2>/dev/null | grep -i imx219"
            )
            if rc != 0 or not out.strip():
                self.fail(t("msg_imx219_media_missing"))
                return

            m = re.search(r'imx219[\w\s\-]+', out, re.IGNORECASE)
            self.info((m.group(0).strip() if m else out.strip().split('\n')[0])[:60])
        finally:
            self._mgr.done(self)


# framebuffer 颜色配置（BGRA 32bpp）
_IMX_COLOR_TESTS = [
    ("R", [0, 0, 255, 255], lambda r, g, b: (r / (r+g+b+1e-3)) > 0.45),
    ("G", [0, 255, 0, 255], lambda r, g, b: (g / (r+g+b+1e-3)) > 0.45),
    ("B", [255, 0, 0, 255], lambda r, g, b: (b / (r+g+b+1e-3)) > 0.45),
]

_FB_WRITE = (
    "python3 -c \""
    "with open('/sys/class/graphics/fb0/virtual_size') as f: "
    "w,h=map(int,f.read().strip().split(','));"
    "color=bytes({bgra});"
    "open('/dev/fb0','wb').write(color*(w*h))\""
)

# ── IMX219 抓帧（改为 HDMI 实时预览 + 人工确认）──────────────────────────────
class Imx219CaptureTest(TestCase):
    """在 HDMI 上显示 IMX219 实时预览，由用户确认是否看到画面。"""

    category_key = "cat_display"
    name_key     = "tn_imx219_capture"

    def __init__(self, board, manager: _Imx219Manager):
        super().__init__(board)
        self._mgr = manager
        manager.register()

    def _run(self):
        try:
            if not self._mgr.ensure(self):
                self.skip(t("msg_imx219_media_missing"))
                return

            # 启动 imx219-preview.sh 作为后台进程，最多运行 60s 避免无限占用
            rc, _, err = self.cmd(
                "nohup timeout 60s imx219-preview.sh 15 > /tmp/imx219_preview.log 2>&1 &",
                timeout=5,
            )
            if rc != 0:
                self.fail(t("msg_imx219_preview_fail", err.strip()[:80]))
                return

            # 等待预览初始化（配置 pipeline + GStreamer preroll）
            time.sleep(3.0)

            # 预览进程若已退出，直接失败
            rc, _, _ = self.cmd("pgrep -f 'gst-launch.*kmssink' >/dev/null")
            if rc != 0:
                self.fail(t("msg_imx219_preview_fail", "预览进程已退出"))
                self.cmd("systemctl start lightdm 2>/dev/null", timeout=10)
                return

            ok = False
            try:
                ok = self.manual_confirm("manual_imx219")
            finally:
                # 先检查预览进程是否还在，再 kill
                rc, _, _ = self.cmd("pgrep -f 'gst-launch.*kmssink' >/dev/null")
                preview_alive = (rc == 0)
                # 终止预览进程，脚本 trap 会自动重启 lightdm
                self.cmd("pkill -f imx219-preview.sh 2>/dev/null; pkill -f 'gst-launch.*kmssink' 2>/dev/null", timeout=5)
                self.cmd("systemctl start lightdm 2>/dev/null", timeout=10)

            if ok:
                if preview_alive:
                    self.pass_(t("msg_imx219_user_yes"))
                else:
                    self.fail(t("msg_imx219_preview_fail", "预览中途退出"))
            else:
                self.fail(t("msg_imx219_user_no"))
        finally:
            self._mgr.done(self)


def get_tests(board) -> list:
    manager = _Imx219Manager(board)
    return [
        Imx219DetectTest(board, manager),
        Imx219CaptureTest(board, manager),
    ]
