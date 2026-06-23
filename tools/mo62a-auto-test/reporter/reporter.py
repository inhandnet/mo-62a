"""HTML 测试报告生成器 — 支持图片 base64 内嵌与双语切换"""

from __future__ import annotations

import base64
import mimetypes
from dataclasses import dataclass, field
from datetime import datetime
from pathlib import Path

from config.i18n import t


@dataclass
class TestRow:
    name:     str
    status:   str
    message:  str
    duration: float
    images:   list = field(default_factory=list)


_STATUS_COLOR = {
    "PASS": "#3fb950",
    "FAIL": "#f85149",
    "INFO": "#00d4ff",
    "SKIP": "#8b949e",
}


_HTML_TEMPLATE = """\
<!DOCTYPE html>
<html lang="{lang}">
<head>
<meta charset="utf-8">
<title>{title}</title>
<style>
  body {{ margin:0; font-family:'Segoe UI',Arial,sans-serif;
          background:#0d1117; color:#e6edf3; }}
  .header {{ background:#161b22; padding:24px 32px;
             border-bottom:1px solid #21262d; }}
  .header h1 {{ margin:0; font-size:20px; color:#00d4ff;
                font-family:monospace; letter-spacing:2px; }}
  .meta {{ margin-top:8px; font-size:12px; color:#8b949e; }}
  .meta span {{ margin-right:24px; }}
  .summary {{ display:flex; gap:20px; padding:16px 32px;
              background:#161b22; border-bottom:1px solid #21262d; }}
  .badge {{ padding:4px 16px; border-radius:20px;
            font-size:13px; font-weight:bold; }}
  .badge-info {{ background:#00d4ff22; color:#00d4ff;
                 border:1px solid #00d4ff55; }}
  .badge-pass {{ background:#3fb95022; color:#3fb950;
                 border:1px solid #3fb95055; }}
  .badge-fail {{ background:#f8514922; color:#f85149;
                 border:1px solid #f8514955; }}
  .badge-skip {{ background:#8b949e22; color:#8b949e;
                 border:1px solid #8b949e55; }}
  .total-dur {{ margin-left:auto; color:#8b949e; font-size:13px; }}
  table {{ width:100%; border-collapse:collapse;
           margin:24px 0; }}
  th {{ background:#161b22; color:#8b949e; font-size:12px;
        padding:10px 16px; text-align:left;
        border-bottom:1px solid #21262d; }}
  td {{ padding:10px 16px; border-bottom:1px solid #21262d;
        font-size:13px; vertical-align:top; }}
  tr:hover td {{ background:#161b22; }}
  .status {{ font-weight:bold; font-family:monospace; }}
  .dur {{ color:#8b949e; font-family:monospace; font-size:12px; }}
  .wrap {{ padding:0 32px 32px; }}
  .gallery {{ margin-top:8px; display:flex; gap:8px; flex-wrap:wrap; }}
  .gallery img {{ max-width:220px; max-height:180px;
                  border-radius:4px; border:1px solid #30363d;
                  cursor:pointer; transition:transform 0.2s; }}
  .gallery img:hover {{ transform:scale(1.05); border-color:#00d4ff; }}
  /* 简易点击放大：fullscreen on click */
  .gallery img:active {{ max-width:90vw; max-height:80vh; }}
</style>
</head>
<body>
<div class="header">
  <h1>◈  {title}</h1>
  <div class="meta">
    <span>{lbl_device}: <b>{hostname}</b></span>
    <span>{lbl_ip}: <b>{ip}</b></span>
    <span>{lbl_time}: <b>{report_time}</b></span>
  </div>
</div>
<div class="summary">
  <span class="badge badge-info">INFO {cnt_info}</span>
  <span class="badge badge-pass">PASS {cnt_pass}</span>
  <span class="badge badge-fail">FAIL {cnt_fail}</span>
  <span class="badge badge-skip">SKIP {cnt_skip}</span>
  <span class="total-dur">{lbl_total}: {total_duration:.2f} s</span>
</div>
<div class="wrap">
<table>
  <thead>
    <tr>
      <th>{col_test}</th>
      <th>{col_status}</th>
      <th>{col_message}</th>
      <th>{col_duration}</th>
    </tr>
  </thead>
  <tbody>
{rows}
  </tbody>
</table>
</div>
</body>
</html>
"""

_ROW_TEMPLATE = """\
    <tr>
      <td>{name}</td>
      <td class="status" style="color:{color}">{status}</td>
      <td>{message}{gallery}</td>
      <td class="dur">{duration:.2f} s</td>
    </tr>"""


def _img_to_data_uri(path: Path) -> str | None:
    """读取图片并转 base64 data URI；失败返回 None。"""
    try:
        if not path.exists():
            return None
        data = path.read_bytes()
        mime, _ = mimetypes.guess_type(path.name)
        mime = mime or "image/png"
        b64 = base64.b64encode(data).decode("ascii")
        return f"data:{mime};base64,{b64}"
    except Exception:
        return None


def _gallery_html(images: list) -> str:
    if not images:
        return ""
    imgs = []
    for p in images:
        uri = _img_to_data_uri(Path(p))
        if uri:
            imgs.append(f'<img src="{uri}" alt="{Path(p).name}" title="{Path(p).name}">')
    if not imgs:
        return ""
    return '<div class="gallery">' + "".join(imgs) + "</div>"


class Reporter:
    def __init__(self, device_info: dict, lang: str = "zh"):
        self._device_info = device_info
        self._lang = lang
        self._rows: list[TestRow] = []

    def add(self, name: str, status: str, message: str,
            duration: float, images: list = None) -> None:
        self._rows.append(
            TestRow(name, status, message, duration, list(images or []))
        )

    def save(self, path: str | Path) -> Path:
        path = Path(path)
        path.parent.mkdir(parents=True, exist_ok=True)

        cnt = {s: 0 for s in ("INFO", "PASS", "FAIL", "SKIP")}
        for r in self._rows:
            cnt[r.status] = cnt.get(r.status, 0) + 1

        total_duration = sum(r.duration for r in self._rows)

        rows_html = "\n".join(
            _ROW_TEMPLATE.format(
                name=r.name,
                color=_STATUS_COLOR.get(r.status, "#e6edf3"),
                status=r.status,
                message=r.message.replace("<", "&lt;").replace(">", "&gt;"),
                gallery=_gallery_html(r.images),
                duration=r.duration,
            )
            for r in self._rows
        )

        html = _HTML_TEMPLATE.format(
            lang           = self._lang,
            title          = t("report_title"),
            lbl_device     = t("report_device"),
            lbl_ip         = t("report_ip"),
            lbl_time       = t("report_time"),
            lbl_total      = t("report_total_duration"),
            hostname       = self._device_info.get("hostname", "—"),
            ip             = self._device_info.get("ip", "—"),
            report_time    = datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            cnt_info       = cnt["INFO"],
            cnt_pass       = cnt["PASS"],
            cnt_fail       = cnt["FAIL"],
            cnt_skip       = cnt["SKIP"],
            total_duration = total_duration,
            col_test       = t("report_col_test"),
            col_status     = t("report_col_status"),
            col_message    = t("report_col_message"),
            col_duration   = t("report_col_duration"),
            rows           = rows_html,
        )
        path.write_text(html, encoding="utf-8")
        return path
