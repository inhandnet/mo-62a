"""
系统服务测试模块
"""

from tests.base import TestCase, TestResult


class NginxServiceTest(TestCase):
    category = "系统服务"
    name_key = "tn_nginx_service"

    def _run(self):
        rc, out, err = self.cmd("systemctl is-active nginx")
        status = out.strip()
        if status != "active":
            self.fail(f"nginx 服务未运行（状态：{status}）")
            return
        self.pass_("nginx 服务正在运行")


class NginxHttpTest(TestCase):
    category = "系统服务"
    name_key = "tn_nginx_http"

    def _run(self):
        rc, out, err = self.cmd(
            'curl -s -o /dev/null -w "%{http_code}" http://localhost/',
            timeout=10,
        )
        code = out.strip()
        if "200" not in code:
            self.fail(f"nginx HTTP 响应码不是 200（实际：{code}）")
            return
        self.pass_(f"nginx HTTP 响应码：{code}")


class LightdmServiceTest(TestCase):
    category = "系统服务"
    name_key = "tn_lightdm_service"

    def _run(self):
        rc, out, err = self.cmd("systemctl is-active lightdm")
        if "active" not in out:
            self.fail(f"lightdm 服务未运行（状态：{out.strip()}）")
            return
        self.pass_(f"lightdm 服务状态：{out.strip()}")


class NginxLogDirTest(TestCase):
    category = "系统服务"
    name_key = "tn_nginx_log"

    def _run(self):
        rc, out, err = self.cmd("ls /var/log/nginx/")
        if rc != 0:
            self.fail(f"/var/log/nginx/ 目录不存在或无法访问：{err.strip()}")
            return
        self.pass_(f"nginx 日志目录存在，内容：{out.strip()}")


class NoFailedServicesTest(TestCase):
    category = "系统服务"
    name_key = "tn_no_failed_services"

    def _run(self):
        rc, out, err = self.cmd(
            "systemctl --failed --no-legend 2>/dev/null | wc -l"
        )
        count_str = out.strip()
        try:
            count = int(count_str)
        except ValueError:
            count = -1

        if count != 0:
            # 获取失败服务列表
            rc2, out2, err2 = self.cmd(
                "systemctl --failed --no-legend 2>/dev/null"
            )
            failed_list = out2.strip() or "(无法获取列表)"
            self.fail(f"存在 {count} 个失败服务：\n{failed_list}")
            return
        self.pass_("无失败的 systemd 服务")


class DpmsWakeupServiceTest(TestCase):
    category = "系统服务"
    name_key = "tn_dpms_wake"

    def _run(self):
        rc, out, err = self.cmd("pgrep -f dpms-wakeup")
        if rc != 0:
            self.fail("dpms-wakeup 进程未运行（pgrep 返回非 0）")
            return
        pids = out.strip()
        self.pass_(f"dpms-wakeup 进程正在运行（PID：{pids}）")


def get_tests(board, manual_confirm_fn=None):
    return [
        NginxServiceTest(board, manual_confirm_fn),
        NginxHttpTest(board, manual_confirm_fn),
        LightdmServiceTest(board, manual_confirm_fn),
        NginxLogDirTest(board, manual_confirm_fn),
        NoFailedServicesTest(board, manual_confirm_fn),
        DpmsWakeupServiceTest(board, manual_confirm_fn),
    ]
