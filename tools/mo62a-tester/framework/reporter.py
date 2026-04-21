"""
HTML 测试报告生成器（纯 Python 字符串拼接，不依赖 jinja2）。
"""

import html
import os
import stat
from datetime import datetime

from gui.i18n import t

# 状态对应的颜色和 i18n key
_STATUS_STYLE = {
    "PASS":        {"bg": "#d4edda", "color": "#155724", "key": "status_pass"},
    "FAIL":        {"bg": "#f8d7da", "color": "#721c24", "key": "status_fail"},
    "SKIP":        {"bg": "#e2e3e5", "color": "#383d41", "key": "status_skip"},
    "INFO":        {"bg": "#e8f4fd", "color": "#0c5460", "key": "status_info"},
    "MANUAL_PASS": {"bg": "#cce5ff", "color": "#004085", "key": "status_manual_pass"},
    "MANUAL_FAIL": {"bg": "#fff3cd", "color": "#856404", "key": "status_manual_fail"},
    "ERROR":       {"bg": "#f8d7da", "color": "#721c24", "key": "status_error"},
}

_DEFAULT_STYLE = {"bg": "#f8f9fa", "color": "#343a40", "key": "rpt_badge_unknown"}


class Reporter:
    """收集测试结果并生成 HTML 报告。"""

    def __init__(self, device_info: dict):
        """初始化报告器。

        Args:
            device_info: 设备信息字典，期望字段：
                hostname, ip, version, build_date, test_time
        """
        self.device_info = device_info
        self.results: list[dict] = []

    # ------------------------------------------------------------------
    # 公共接口
    # ------------------------------------------------------------------

    def add_result(
        self,
        category: str,
        name: str,
        status: str,
        message: str = "",
        duration: float = 0.0,
    ) -> None:
        """追加一条测试结果。

        Args:
            category: 测试大类（如"系统基础"）
            name: 测试项名称
            status: "PASS" | "FAIL" | "SKIP" | "MANUAL_PASS" | "MANUAL_FAIL"
            message: 附加说明（可选）
            duration: 耗时（秒）
        """
        self.results.append(
            {
                "category": category,
                "name": name,
                "status": status,
                "message": message,
                "duration": duration,
            }
        )

    def summary(self) -> dict:
        total = len(self.results)
        passed = sum(
            1 for r in self.results if r["status"] in ("PASS", "MANUAL_PASS", "INFO")
        )
        failed = sum(
            1 for r in self.results if r["status"] in ("FAIL", "MANUAL_FAIL", "ERROR")
        )
        skipped = sum(1 for r in self.results if r["status"] == "SKIP")
        return {"total": total, "passed": passed, "failed": failed, "skipped": skipped}

    def save(self, filepath: str) -> None:
        """生成 HTML 报告并保存到文件。

        Args:
            filepath: 输出文件路径（绝对路径或相对路径均可）
        """
        html_content = self._build_html()
        with open(filepath, "w", encoding="utf-8") as f:
            f.write(html_content)
        # 确保文件对普通用户可读（sudo 运行时文件会被 root 创建）
        try:
            os.chmod(filepath, stat.S_IRUSR | stat.S_IWUSR | stat.S_IRGRP | stat.S_IROTH)
            uid = int(os.environ.get("SUDO_UID", os.getuid()))
            gid = int(os.environ.get("SUDO_GID", os.getgid()))
            os.chown(filepath, uid, gid)
        except OSError:
            pass

    # ------------------------------------------------------------------
    # 内部构建方法
    # ------------------------------------------------------------------

    def _build_html(self) -> str:
        summ = self.summary()
        generated_at = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

        # 按 category 分组
        categories: dict[str, list[dict]] = {}
        for r in self.results:
            categories.setdefault(r["category"], []).append(r)

        pass_rate = (
            f"{summ['passed'] / summ['total'] * 100:.1f}%" if summ["total"] > 0 else "N/A"
        )

        sections = "\n".join(
            self._build_category_section(cat, items)
            for cat, items in categories.items()
        )

        title = t("rpt_title") + " — " + self.device_info.get("hostname", "N/A")

        return f"""<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>{html.escape(title)}</title>
<style>
  /* ===== Reset & Base ===== */
  *, *::before, *::after {{ box-sizing: border-box; margin: 0; padding: 0; }}
  body {{
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto,
                 "Helvetica Neue", Arial, "Noto Sans CJK SC", sans-serif;
    font-size: 14px;
    line-height: 1.6;
    color: #212529;
    background: #f0f2f5;
  }}

  /* ===== Layout ===== */
  .page-wrapper {{
    max-width: 1100px;
    margin: 0 auto;
    padding: 24px 16px;
  }}

  /* ===== Header ===== */
  .report-header {{
    background: linear-gradient(135deg, #1a237e 0%, #283593 60%, #3949ab 100%);
    color: #fff;
    border-radius: 12px;
    padding: 32px 36px;
    margin-bottom: 24px;
    box-shadow: 0 4px 20px rgba(26,35,126,0.3);
  }}
  .report-header h1 {{
    font-size: 24px;
    font-weight: 700;
    letter-spacing: 0.5px;
    margin-bottom: 6px;
  }}
  .report-header .subtitle {{
    font-size: 13px;
    opacity: 0.8;
  }}

  /* ===== Summary Cards ===== */
  .summary-grid {{
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
    gap: 16px;
    margin-bottom: 24px;
  }}
  .summary-card {{
    background: #fff;
    border-radius: 10px;
    padding: 20px 16px;
    text-align: center;
    box-shadow: 0 2px 8px rgba(0,0,0,0.08);
    border-top: 4px solid transparent;
  }}
  .summary-card .card-value {{
    font-size: 36px;
    font-weight: 800;
    line-height: 1;
    margin-bottom: 6px;
  }}
  .summary-card .card-label {{
    font-size: 13px;
    color: #6c757d;
    font-weight: 500;
  }}
  .card-total   {{ border-top-color: #6c757d; }}
  .card-total   .card-value {{ color: #495057; }}
  .card-passed  {{ border-top-color: #28a745; }}
  .card-passed  .card-value {{ color: #28a745; }}
  .card-failed  {{ border-top-color: #dc3545; }}
  .card-failed  .card-value {{ color: #dc3545; }}
  .card-skipped {{ border-top-color: #6c757d; }}
  .card-skipped .card-value {{ color: #6c757d; }}
  .card-rate    {{ border-top-color: #007bff; }}
  .card-rate    .card-value {{ color: #007bff; font-size: 28px; }}

  /* ===== Device Info ===== */
  .device-card {{
    background: #fff;
    border-radius: 10px;
    padding: 20px 24px;
    margin-bottom: 24px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.08);
  }}
  .device-card h2 {{
    font-size: 16px;
    font-weight: 600;
    color: #343a40;
    margin-bottom: 14px;
    padding-bottom: 10px;
    border-bottom: 2px solid #e9ecef;
  }}
  .device-table {{
    width: 100%;
    border-collapse: collapse;
  }}
  .device-table td {{
    padding: 7px 12px;
    font-size: 13px;
  }}
  .device-table td:first-child {{
    font-weight: 600;
    color: #495057;
    width: 160px;
    white-space: nowrap;
  }}
  .device-table tr:nth-child(even) td {{
    background: #f8f9fa;
  }}

  /* ===== Test Sections ===== */
  .test-section {{
    background: #fff;
    border-radius: 10px;
    margin-bottom: 20px;
    box-shadow: 0 2px 8px rgba(0,0,0,0.08);
    overflow: hidden;
  }}
  .section-header {{
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 14px 20px;
    background: #f8f9fa;
    border-bottom: 1px solid #e9ecef;
  }}
  .section-title {{
    font-size: 15px;
    font-weight: 700;
    color: #343a40;
  }}
  .section-badge {{
    font-size: 12px;
    font-weight: 600;
    padding: 3px 10px;
    border-radius: 20px;
    background: #e9ecef;
    color: #495057;
  }}

  /* ===== Results Table ===== */
  .results-table {{
    width: 100%;
    border-collapse: collapse;
  }}
  .results-table thead th {{
    background: #f1f3f5;
    padding: 10px 14px;
    text-align: left;
    font-size: 12px;
    font-weight: 700;
    text-transform: uppercase;
    letter-spacing: 0.5px;
    color: #6c757d;
    border-bottom: 2px solid #dee2e6;
  }}
  .results-table tbody td {{
    padding: 10px 14px;
    border-bottom: 1px solid #f1f3f5;
    font-size: 13px;
    vertical-align: top;
  }}
  .results-table tbody tr:last-child td {{
    border-bottom: none;
  }}
  .results-table tbody tr:hover td {{
    background: #f8f9fa;
  }}
  .col-name    {{ width: 28%; font-weight: 500; }}
  .col-status  {{ width: 12%; text-align: center; }}
  .col-dur     {{ width: 10%; text-align: right; color: #6c757d; }}
  .col-message {{ width: 50%; word-break: break-word; }}

  /* ===== Status Badge ===== */
  .badge {{
    display: inline-block;
    padding: 3px 10px;
    border-radius: 20px;
    font-size: 11px;
    font-weight: 700;
    letter-spacing: 0.3px;
    white-space: nowrap;
  }}

  /* ===== Footer ===== */
  .report-footer {{
    text-align: center;
    font-size: 12px;
    color: #adb5bd;
    margin-top: 16px;
    padding: 8px 0;
  }}
</style>
</head>
<body>
<div class="page-wrapper">

  <!-- Header -->
  <div class="report-header">
    <h1>{html.escape(title)}</h1>
    <div class="subtitle">
      {html.escape(t("rpt_generated_at", generated_at))} &nbsp;|&nbsp;
      {html.escape(t("rpt_test_time", str(self.device_info.get("test_time", "N/A"))))}
    </div>
  </div>

  <!-- Summary Cards -->
  <div class="summary-grid">
    <div class="summary-card card-total">
      <div class="card-value">{summ['total']}</div>
      <div class="card-label">{html.escape(t("rpt_total"))}</div>
    </div>
    <div class="summary-card card-passed">
      <div class="card-value">{summ['passed']}</div>
      <div class="card-label">{html.escape(t("rpt_passed"))}</div>
    </div>
    <div class="summary-card card-failed">
      <div class="card-value">{summ['failed']}</div>
      <div class="card-label">{html.escape(t("rpt_failed"))}</div>
    </div>
    <div class="summary-card card-skipped">
      <div class="card-value">{summ['skipped']}</div>
      <div class="card-label">{html.escape(t("rpt_skipped"))}</div>
    </div>
    <div class="summary-card card-rate">
      <div class="card-value">{pass_rate}</div>
      <div class="card-label">{html.escape(t("rpt_pass_rate"))}</div>
    </div>
  </div>

  <!-- Device Info -->
  <div class="device-card">
    <h2>{html.escape(t("rpt_device_info"))}</h2>
    <table class="device-table">
      <tbody>
        <tr><td>{html.escape(t("rpt_hostname"))}</td><td>{html.escape(str(self.device_info.get("hostname", "N/A")))}</td></tr>
        <tr><td>{html.escape(t("rpt_ip"))}</td><td>{html.escape(str(self.device_info.get("ip", "N/A")))}</td></tr>
        <tr><td>{html.escape(t("rpt_version"))}</td><td>{html.escape(str(self.device_info.get("version", "N/A")))}</td></tr>
        <tr><td>{html.escape(t("rpt_build_date"))}</td><td>{html.escape(str(self.device_info.get("build_date", "N/A")))}</td></tr>
        <tr><td>{html.escape(t("rpt_duration"))}</td><td>{html.escape(str(self.device_info.get("test_time", "N/A")))}</td></tr>
      </tbody>
    </table>
  </div>

  <!-- Test Results -->
  {sections}

  <div class="report-footer">
    {html.escape(t("rpt_footer", generated_at))}
  </div>

</div>
</body>
</html>"""

    def _build_category_section(self, category: str, items: list[dict]) -> str:
        cat_pass = sum(1 for r in items if r["status"] in ("PASS", "MANUAL_PASS", "INFO"))
        cat_fail = sum(1 for r in items if r["status"] in ("FAIL", "MANUAL_FAIL", "ERROR"))
        badge_text = t("rpt_cat_pass", cat_pass, len(items))
        badge_extra = t("rpt_cat_fail", cat_fail) if cat_fail else ""

        rows = "\n".join(self._build_row(r) for r in items)

        return f"""  <div class="test-section">
    <div class="section-header">
      <span class="section-title">{html.escape(category)}</span>
      <span class="section-badge">{html.escape(badge_text + badge_extra)}</span>
    </div>
    <table class="results-table">
      <thead>
        <tr>
          <th class="col-name">{html.escape(t("rpt_col_name"))}</th>
          <th class="col-status">{html.escape(t("rpt_col_status"))}</th>
          <th class="col-dur">{html.escape(t("rpt_col_dur"))}</th>
          <th class="col-message">{html.escape(t("rpt_col_message"))}</th>
        </tr>
      </thead>
      <tbody>
        {rows}
      </tbody>
    </table>
  </div>"""

    def _build_row(self, result: dict) -> str:
        """构建单行测试结果 HTML。"""
        style = _STATUS_STYLE.get(result["status"], _DEFAULT_STYLE)
        badge_html = (
            f'<span class="badge" style="background:{style["bg"]};color:{style["color"]}">'
            f'{html.escape(t(style["key"]))}</span>'
        )
        duration_str = f'{result["duration"]:.2f}s' if result["duration"] else "—"
        message_escaped = html.escape(result.get("message", "")).replace("\n", "<br>")

        return (
            f'<tr>'
            f'<td class="col-name">{html.escape(result["name"])}</td>'
            f'<td class="col-status">{badge_html}</td>'
            f'<td class="col-dur">{duration_str}</td>'
            f'<td class="col-message">{message_escaped}</td>'
            f'</tr>'
        )
