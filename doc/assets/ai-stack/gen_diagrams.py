#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""生成《TI AM62A 边缘 AI 软件栈详解》的配图（SVG → PNG）。

用法:
    python3 gen_diagrams.py && ./render.sh

说明:
    - 不使用 emoji（rsvg 无法渲染彩色 emoji，会变成空白方块），图标一律用矢量图形绘制。
    - 所有文字块统一走 card() 的多行布局，避免文字互相压盖。
    - 最终以 2x 缩放渲染 PNG，保证在文档中清晰。
"""
import os

OUT = os.path.dirname(os.path.abspath(__file__))
F = "Noto Sans CJK SC, DejaVu Sans, sans-serif"
FM = "DejaVu Sans Mono, Noto Sans Mono CJK SC, monospace"

C = {
    "bg": "#f5f7fb", "ink": "#1f2437", "muted": "#6b7280", "line": "#c7cddb",
    "app":     ("#5b8def", "#3f66c9"),
    "fw":      ("#8b7bd8", "#6a58bd"),
    "backend": ("#a86bd0", "#8442ae"),
    "rt":      ("#6c5ce7", "#4834d4"),
    "vx":      ("#4834d4", "#2d1f96"),
    "ipc":     ("#f0932b", "#cf7714"),
    "fwc":     ("#d63bd6", "#a029a0"),
    "hw":      ("#e74c3c", "#bf3327"),
    "vpac":    ("#ff9f43", "#df801f"),
    "mem":     ("#16a085", "#0c7862"),
    "ok":      ("#27ae60", "#1c8449"),
    "warn":    ("#eb4d4b", "#bf3327"),
    "gray":    ("#9aa5b1", "#7b8794"),
    "dark":    ("#3d566e", "#2b3f52"),
    "sram":    ("#22a6b3", "#12838f"),
    "gold":    ("#e1b12c", "#bd9218"),
    "slate":   ("#7f8fa6", "#64748b"),
}


def head(w, h, title=None, sub=None):
    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}">', "<defs>"]
    for k, v in C.items():
        if isinstance(v, tuple):
            s.append(f'<linearGradient id="g_{k}" x1="0" y1="0" x2="0.55" y2="1">'
                     f'<stop offset="0" stop-color="{v[0]}"/><stop offset="1" stop-color="{v[1]}"/></linearGradient>')
    s += ['<filter id="sh" x="-30%" y="-30%" width="160%" height="190%">'
          '<feDropShadow dx="0" dy="3" stdDeviation="4" flood-color="#16204a" flood-opacity="0.20"/></filter>',
          '<filter id="shs" x="-30%" y="-30%" width="160%" height="190%">'
          '<feDropShadow dx="0" dy="1.5" stdDeviation="2" flood-color="#16204a" flood-opacity="0.15"/></filter>',
          '<marker id="ar" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">'
          '<path d="M0,1 L9,5 L0,9 z" fill="#5a6480"/></marker>',
          '<marker id="arw" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">'
          '<path d="M0,1 L9,5 L0,9 z" fill="#ffffff"/></marker>',
          '<marker id="arr" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">'
          '<path d="M0,1 L9,5 L0,9 z" fill="#eb4d4b"/></marker>',
          '<marker id="aro" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">'
          '<path d="M0,1 L9,5 L0,9 z" fill="#e08424"/></marker>',
          '<marker id="arg" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">'
          '<path d="M0,1 L9,5 L0,9 z" fill="#27ae60"/></marker>',
          "</defs>", f'<rect width="{w}" height="{h}" fill="{C["bg"]}"/>']
    if title:
        s.append(txt(w / 2, 42, title, 26, C["ink"], "middle", "700"))
    if sub:
        s.append(txt(w / 2, 68, sub, 14, C["muted"], "middle"))
    return s


def esc(t):
    return t.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def txt(x, y, t, size=14, fill="#1f2437", anchor="start", weight="400", font=None, op=1.0):
    return (f'<text x="{x}" y="{y}" font-family="{font or F}" font-size="{size}" fill="{fill}" '
            f'text-anchor="{anchor}" font-weight="{weight}" opacity="{op}">{esc(t)}</text>')


def box(x, y, w, h, fill, r=13, shadow=True, op=1.0, stroke=None, sw=1.5, dash=None):
    g = f'url(#g_{fill})' if fill in C and isinstance(C[fill], tuple) else fill
    st = f' stroke="{stroke}" stroke-width="{sw}"' if stroke else ""
    if dash:
        st += f' stroke-dasharray="{dash}"'
    return (f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{g}" opacity="{op}"{st}'
            f'{" filter=\"url(#sh)\"" if shadow else ""}/>')


def card(x, y, w, h, lines, fill="app", r=13, align="middle", pad=16, shadow=True):
    """lines: [(text, size, weight, opacity), ...] —— 整块垂直居中，绝不重叠"""
    o = [box(x, y, w, h, fill, r=r, shadow=shadow)]
    norm = []
    for ln in lines:
        t = ln[0]
        sz = ln[1] if len(ln) > 1 else 14
        wt = ln[2] if len(ln) > 2 else "400"
        op = ln[3] if len(ln) > 3 else 1.0
        norm.append((t, sz, wt, op))
    gap = 6
    total = sum(n[1] for n in norm) + gap * (len(norm) - 1)
    cy = y + h / 2 - total / 2
    cx = x + w / 2 if align == "middle" else x + pad
    for t, sz, wt, op in norm:
        cy += sz
        o.append(txt(cx, cy, t, sz, "#ffffff", align, wt, op=op))
        cy += gap
    return o


def panel(x, y, w, h, label=None, sub=None, fill="#ffffff", stroke="#dde3ee", r=15):
    o = [f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{r}" fill="{fill}" stroke="{stroke}" stroke-width="1.5"/>']
    if label:
        o.append(txt(x + 18, y + 26, label, 15, C["ink"], weight="700"))
    if sub:
        o.append(txt(x + 18 + len(label) * 15.5, y + 26, sub, 11.5, C["muted"]))
    return o


def arrow(x1, y1, x2, y2, color="#5a6480", w=2.2, dash=None, marker="ar"):
    d = f' stroke-dasharray="{dash}"' if dash else ""
    return (f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{color}" stroke-width="{w}" '
            f'marker-end="url(#{marker})" stroke-linecap="round"{d}/>')


def apath(d, color="#5a6480", w=2.2, dash=None, marker="ar", fill="none"):
    ds = f' stroke-dasharray="{dash}"' if dash else ""
    m = f' marker-end="url(#{marker})"' if marker else ""
    return f'<path d="{d}" fill="{fill}" stroke="{color}" stroke-width="{w}" stroke-linecap="round" stroke-linejoin="round"{m}{ds}/>'


def pill(x, y, t, fill="#eb4d4b", tc="#fff", size=11.5, h=23):
    cjk = sum(1 for ch in t if ord(ch) > 0x2E80)
    w = (len(t) - cjk) * size * 0.56 + cjk * size * 1.03 + 20
    return [f'<rect x="{x}" y="{y}" width="{w:.0f}" height="{h}" rx="{h/2}" fill="{fill}" filter="url(#shs)"/>',
            txt(x + w / 2, y + h / 2 + 4.2, t, size, tc, "middle", "700")], w


def warn_line(x, y, t, w=1180, size=13):
    """红色警示条"""
    return [f'<rect x="{x}" y="{y}" width="{w}" height="34" rx="9" fill="#fdecea" stroke="#f5b7b1" stroke-width="1.3"/>',
            f'<rect x="{x}" y="{y}" width="5" height="34" rx="2.5" fill="#eb4d4b"/>',
            txt(x + 18, y + 22, t, size, "#a93226", weight="700")]


def save(name, parts):
    parts.append("</svg>")
    with open(os.path.join(OUT, name + ".svg"), "w", encoding="utf-8") as f:
        f.write("\n".join(parts))
    print("  ✓", name)


# ══════════════════════ 01 全景分层 ══════════════════════
def d01():
    W, H = 1320, 950
    s = head(W, H, "TI AM62A 边缘 AI 软件栈 · 全景", "Mo 62A 实机 · 从应用到硅片的完整调用链")
    L, BW = 60, 1010
    R = L + BW
    cw = (BW - 60) / 3

    def row3(y, items, fill):
        o = []
        for i, ln in enumerate(items):
            o += card(L + 15 + i * (cw + 15), y, cw, 56, ln, fill)
        return o

    y = 96
    s += panel(L, y, BW, 104, "① 应用层", "   用户程序 / 官方 demo")
    s += row3(y + 36, [[("edgeai-gst-apps", 14.5, "700"), ("TI 官方 demo", 11, "400", .85)],
                       [("edgeai-demo", 14.5, "700"), ("自研统一启动器", 11, "400", .85)],
                       [("你的 C++ 程序", 14.5, "700"), ("find_package(EdgeAI)", 11, "400", .85)]], "app")

    y += 118
    s += panel(L, y, BW, 104, "② 推理框架层", "   三条并行的路，互相独立")
    s += row3(y + 36, [[("ONNX Runtime 1.15", 14, "700"), ("必须用 TI 版", 11, "400", .85)],
                       [("TFLite Runtime 2.12", 14, "700"), (".tflite", 11, "400", .85)],
                       [("DLR 1.13", 14, "700"), ("TVM 编译产物", 11, "400", .85)]], "fw")

    y += 118
    s += panel(L, y, BW, 104, "③ TI 后端", "   算子适配 · 决定哪些层能交给 C7x")
    s += row3(y + 36, [[("libtidl_onnxrt_EP", 13.5, "700"), ("TIDLExecutionProvider", 10.5, "400", .85)],
                       [("libtidl_tfl_delegate", 13.5, "700"), ("TfLite Delegate", 10.5, "400", .85)],
                       [("libdlr", 13.5, "700"), ("DLR runtime", 10.5, "400", .85)]], "backend")

    y += 118
    s += card(L, y, BW, 62, [("④   TIDL-RT   ·   libvx_tidl_rt.so", 17, "700"),
                             ("把「跑一个网络」翻译成 OpenVX 计算图", 12, "400", .88)], "rt")
    y += 78
    s += card(L, y, BW, 74, [("⑤   OpenVX / TIOVX 框架   ·   libtivision_apps.so", 17, "700"),
                             ("host 侧节点管理与调度 —— 同时服务推理链与 GStreamer 摄像头插件", 12, "400", .88)], "vx")
    y += 90
    s += card(L, y, BW, 62, [("⑥   IPC 层   ·   rpmsg  +  dma_heap", 16.5, "700"),
                             ("命令走 vring 环形缓冲，数据走共享内存（零拷贝）", 12, "400", .88)], "ipc")
    y += 78
    s += card(L, y, BW, 62, [("⑦   C7x 固件   ·   am62a-c71_0-fw  (11 MB)", 16.5, "700"),
                             ("TIDL 算法库  +  MMALIB 核函数  +  TIOVX target 侧", 12, "400", .88)], "fwc")
    y += 78
    hw = BW * 0.62
    s += card(L, y, hw, 76, [("⑧   C7x DSP + MMA 矩阵阵列", 17.5, "700"),
                             ("唯一执行神经网络计算的单元 · 2 TOPS", 12, "400", .9)], "hw")
    s += card(L + hw + 16, y, BW - hw - 16, 76, [("VPAC", 17, "700"), ("ISP / MSC / LDC", 11.5, "400", .9)], "vpac")

    # 主链箭头
    ax = L + BW / 2
    for y0, y1 in [(200, 214), (318, 332), (436, 450), (512, 526), (590, 604), (680, 694), (758, 772)]:
        s.append(arrow(ax, y0, ax, y1, "#8b93ab", 2.6))
    s.append(apath(f"M {L+hw+16+(BW-hw-16)/2} 758 L {L+hw+16+(BW-hw-16)/2} 772", "#e08424", 2.4, dash="5 4", marker="aro"))

    # GStreamer 旁路
    gx = R + 26
    s.append(apath(f"M {R} 124 C {gx+120} 124, {gx+120} 600, {R} 600", "#e08424", 2.8, marker="aro"))
    s += card(R + 34, 300, 200, 96, [("GStreamer", 15, "700"), ("TIOVX 插件", 15, "700"),
                                     ("tiovxisp / multiscaler", 10.5, "400", .88),
                                     ("绕过推理框架直连", 10.5, "400", .88)], "vpac")

    s += warn_line(L, H - 56, "版本铁律：⑦固件 · ⑤libtivision_apps · ④libvx_tidl_rt · ③EP 库 · 模型 artifacts —— 五者必须同版本，错配报错极具误导性", w=BW + 234)
    save("01-overview", s)


# ══════════════════════ 02 硬件框图 ══════════════════════
def d02():
    W, H = 1180, 720
    s = head(W, H, "AM62A7 SoC 硬件框图", "哪些单元参与 AI，哪些不参与")

    s += panel(55, 92, 830, 494, "AM62A7 SoC", "   SR1.0 · Func-Safe 'U' Grade", stroke="#b9c1d4")

    s += card(85, 150, 370, 100, [("4 × Cortex-A53", 20, "700"),
                                  ("运行 Linux 6.12.35 PREEMPT_RT", 12, "400", .9),
                                  ("1.4 GHz · 7 档 DVFS", 12, "400", .78)], "dark")
    s += card(485, 150, 370, 100, [("C7x DSP  +  MMA", 20, "700"),
                                   ("唯一执行神经网络计算的单元", 12, "400", .92),
                                   ("L1D 32K · L2 256K · 2 TOPS", 12, "400", .8)], "hw")

    yy = 282
    s += card(85, yy, 230, 92, [("VPAC", 17, "700"), ("ISP · MSC×1 · LDC", 11.5, "400", .92),
                                ("MSC 只有一个实例", 11, "400", .8)], "vpac")
    s += card(331, yy, 190, 92, [("VPU", 17, "700"), ("H.264/265 编解码", 11.5, "400", .92),
                                 ("不参与 AI", 11, "400", .8)], "gray")
    s += card(537, yy, 155, 92, [("MCU R5F", 15.5, "700"), ("IPC echo", 11, "400", .9),
                                 ("54 KB 固件", 10.5, "400", .78)], "gray")
    s += card(708, yy, 147, 92, [("DM R5F", 15.5, "700"), ("电源 / 时钟", 11, "400", .9),
                                 ("k3conf 问它", 10.5, "400", .78)], "gray")

    s += card(85, 410, 770, 72, [("MSMC SRAM   ·   1 MB", 17.5, "700"),
                                 ("C7x 与 A53 共享的高速片上内存", 12, "400", .9)], "sram")

    s.append(arrow(270, 250, 270, 406, "#8b93ab"))
    s.append(arrow(670, 250, 670, 406, "#8b93ab"))
    s.append(arrow(455, 200, 481, 200, "#8b93ab"))
    s.append(txt(468, 190, "控制", 10.5, C["muted"], "middle"))

    s += card(925, 240, 195, 242, [("LPDDR4", 19, "700"), ("2 GB", 26, "700"),
                                   ("3200 MHz", 12, "400", .88), ("", 4),
                                   ("保留 615 MB", 12, "400", .85),
                                   ("可用 1.35 GB", 12, "400", .85)], "mem")
    s.append(arrow(858, 446, 920, 446, "#8b93ab"))

    ly = 626
    s.append(txt(58, ly, "图例：", 13.5, C["ink"], weight="700"))
    lx = 118
    for lab, col in [("AI 计算", "hw"), ("图像/视频硬件", "vpac"), ("与 AI 无关", "gray"), ("共享存储", "sram")]:
        s.append(box(lx, ly - 13, 18, 18, col, r=5, shadow=False))
        s.append(txt(lx + 26, ly, lab, 12.5, C["muted"]))
        lx += 26 + len(lab) * 13.5 + 40

    s += warn_line(55, 660, "AM62A 没有独立 NPU —— 所谓 2 TOPS 就是 C7x + MMA。也没有 RGA（那是瑞芯微的部件）。VPU 只做视频编解码，不能加速推理。", w=1065)
    save("02-hardware", s)


# ══════════════════════ 03 DDR 内存分布 ══════════════════════
def d03():
    """左：2GB 总览（断轴）；右：615MB 保留区等距展开，避免小区块塌缩"""
    W, H = 1280, 760
    s = head(W, H, "DDR 内存分布（实测 2 GB）", "AI 相关区域占 615 MB —— 这就是 free 只看到 1.35 GB 的原因")

    # ── 左：总览条 ──
    bx, by, bw = 70, 116, 230
    s.append(txt(bx, by - 16, "物理内存总览", 13, C["ink"], weight="700"))

    seg = [("保留区", 615, "mem", 300), ("Linux 可用内存", 1379, "gray", 236)]
    yy = by
    marks = []
    for name, mb, col, hh in seg:
        s.append(box(bx, yy, bw, hh, col, r=8, shadow=False))
        s.append(txt(bx + bw / 2, yy + hh / 2 - 4, name, 14, "#fff", "middle", "700"))
        s.append(txt(bx + bw / 2, yy + hh / 2 + 18, f"{mb} MB", 16, "#fff", "middle", "700"))
        marks.append((yy, yy + hh))
        yy += hh
        # 断轴锯齿
        if name == "保留区":
            zig = " ".join(f"{bx + i*23},{yy + (4 if i%2 else -4)}" for i in range(11))
            s.append(f'<polyline points="{zig}" fill="none" stroke="{C["bg"]}" stroke-width="7"/>')
            s.append(f'<polyline points="{zig}" fill="none" stroke="#aab3c5" stroke-width="1.6"/>')
            yy += 10

    s.append(txt(bx, by + 570, "0x8000_0000", 11.5, C["muted"], font=FM))
    s.append(txt(bx, by - 30, "0xC000_0000", 11.5, C["muted"], font=FM))
    s.append(txt(bx + bw / 2, by + 600, "断轴：Linux 区已压缩", 11, C["muted"], "middle"))

    # ── 放大引导线 ──
    ex, ew = 470, 740
    ey, eh = 116, 540
    s.append(apath(f"M {bx+bw} {marks[0][0]} L {ex} {ey}", "#b6bece", 1.6, marker=None, dash="5 4"))
    s.append(apath(f"M {bx+bw} {marks[0][1]} L {ex} {ey+eh}", "#b6bece", 1.6, marker=None, dash="5 4"))

    # ── 右：保留区等距展开 ──
    s.append(f'<rect x="{ex}" y="{ey}" width="{ew}" height="{eh}" rx="12" fill="#ffffff" stroke="#dde3ee" stroke-width="1.5"/>')
    s.append(txt(ex + 20, ey - 16, "保留区 615 MB 展开（按地址从高到低，等距绘制）", 13, C["ink"], weight="700"))

    regions = [
        ("EdgeAI Core Heap", "0xADC0_0000", "292 MB", "mem", "网络权重 · 中间激活值", True),
        ("EdgeAI 共享内存池", "0xA300_0000", "172 MB", "ok", "dma_heap 暴露 · A53↔C7x 零拷贝", True),
        ("EdgeAI DMA", "0xA100_0000", "32 MB", "sram", "DMA 缓冲", True),
        ("IPC 共享内存", "0xA000_0000", "16 MB", "ipc", "跨核 IPC", False),
        ("OP-TEE 安全世界", "0x9E80_0000", "24 MB", "dark", "安全世界", False),
        ("R5F DMA / 固件 ×4", "0x9B80_0000", "47 MB", "slate", "两个 R5F 核", False),
        ("C7x 固件运行区", "0x9990_0000", "31 MB", "fwc", "am62a-c71_0-fw 加载于此", True),
        ("C7x vring", "0x9980_0000", "1 MB", "gold", "IPC 环形缓冲（命令通道）", True),
        ("TF-A", "0x8000_0000", "0.5 MB", "warn", "Arm Trusted Firmware", False),
    ]
    rh = (eh - 40) / len(regions)
    yy = ey + 20
    for name, addr, size, col, desc, ai in regions:
        s.append(box(ex + 18, yy + 3, 6, rh - 12, col, r=3, shadow=False))
        s.append(box(ex + 34, yy + 3, 150, rh - 12, col, r=7, shadow=False))
        s.append(txt(ex + 109, yy + rh / 2 + 2, size, 14, "#fff", "middle", "700"))
        s.append(txt(ex + 200, yy + rh / 2 - 4, name, 13.5, C["ink"], weight="700"))
        s.append(txt(ex + 200, yy + rh / 2 + 15, f"{addr}   ·   {desc}", 11.5, C["muted"]))
        if ai:
            b, bwid = pill(ex + ew - 96, yy + rh / 2 - 12, "AI 相关", "#16a085", size=10.5, h=21)
            s += b
        yy += rh

    s += warn_line(70, H - 60, "部署大模型报 algAlloc failed 时，先怀疑 Core Heap 292 MB 不够，而不是急着怀疑算子不支持 —— DSP 日志会打印网络的真实内存需求。", w=1140)
    save("03-memory-map", s)


# ══════════════════════ 04 存储层次 ══════════════════════
def d04():
    W, H = 1160, 620
    s = head(W, H, "C7x 片上存储层次", "推理性能高度依赖数据能否留在片上 —— TIDL 编译的「内存规划」就是在解这个问题")

    levels = [
        ("L1D  Cache / Scratch", "16 KB", "~1 周期", 250, "ok"),
        ("L2  SRAM", "224 KB", "~10 周期", 370, "sram"),
        ("MSMC  SRAM", "1024 KB", "~30 周期", 510, "ipc"),
        ("DDR (LPDDR4)", "数百 MB", "~100+ 周期", 660, "warn"),
    ]
    CX = 430
    y = 118
    for name, size, lat, w, col in levels:
        s += card(CX - w / 2, y, w, 82, [(f"{name}   {size}", 16, "700"), (f"访问延迟 {lat}", 12, "400", .88)], col)
        y += 96
    for yy in [200, 296, 392]:
        s.append(arrow(CX, yy, CX, yy + 14, "#8b93ab", 2.4))

    s.append(txt(96, 150, "快", 22, "#27ae60", "middle", "700"))
    s.append(txt(96, 470, "慢", 22, "#eb4d4b", "middle", "700"))
    s.append(apath("M 96 168 L 96 448", "#c7cddb", 3, marker="ar"))

    # 右侧配置
    s += panel(820, 118, 300, 274, "编译工具里的硬编码", None)
    cfgs = [("L2MEMSIZE_KB", "224"), ("MSMCSIZE_KB", "1024"), ("DEVICE_NAME", "4 (AM62A)"), ("DDRFREQ_MHZ", "3200")]
    yy = 168
    for k, v in cfgs:
        s.append(txt(840, yy, k, 12, C["muted"], font=FM))
        s.append(txt(1100, yy, v, 12.5, C["ink"], "end", "700", font=FM))
        yy += 28
    s.append(txt(840, yy + 8, "device_config.cfg", 11, C["muted"], font=FM))
    s.append(txt(840, yy + 26, "改它 = 改硬件假设", 11.5, "#a93226", weight="700"))

    s += warn_line(70, H - 62, "实测 DeiT-tiny 内存需求：L1D 16 KB · L2 224 KB · MSMC 1024 KB · DDR scratch 2.7 MB · DDR 权重常驻 17.7 MB（debug_level=2 时 DSP 打印）", w=1020)
    save("04-memory-hierarchy", s)


# ══════════════════════ 05 IPC 通信 ══════════════════════
def d05():
    W, H = 1200, 720
    s = head(W, H, "A53 与 C7x 如何对话", "关键设计：命令走 vring，数据走共享内存 —— 每帧图像不经过 IPC 通道")

    s += panel(60, 100, 300, 330, "A53 / Linux 侧")
    s += card(82, 150, 256, 62, [("推理程序", 15, "700"), ("ONNX Runtime / 你的程序", 10.5, "400", .88)], "app")
    s += card(82, 226, 256, 62, [("libtivision_apps", 14.5, "700"), ("OpenVX host 侧", 10.5, "400", .88)], "vx")
    s += card(82, 302, 256, 62, [("rpmsg 驱动", 14.5, "700"), ("/dev/rpmsg0-4", 10.5, "400", .88)], "slate")
    s.append(arrow(210, 212, 210, 222, "#8b93ab"))
    s.append(arrow(210, 288, 210, 298, "#8b93ab"))

    s += panel(440, 100, 320, 330, "共享内存（物理）")
    s += card(462, 150, 276, 96, [("vring 环形缓冲", 15, "700"), ("0x9980_0000  ·  1 MB", 11, "400", .9),
                                  ("传「命令」", 12, "700", .95)], "gold")
    s += card(462, 268, 276, 118, [("EdgeAI 共享池", 15, "700"), ("0xA300_0000  ·  172 MB", 11, "400", .9),
                                   ("传「数据」· 零拷贝", 12, "700", .95),
                                   ("/dev/dma_heap/carveout_...", 9.5, "400", .8)], "ok")

    s += panel(840, 100, 300, 330, "C7x 侧")
    s += card(862, 150, 256, 62, [("TIOVX target kernel", 13.5, "700"), ("接收节点创建/执行命令", 10.5, "400", .88)], "fwc")
    s += card(862, 268, 256, 118, [("TIDL 算法 + MMALIB", 14, "700"), ("逐层计算", 11, "400", .88),
                                   ("MMA 矩阵阵列", 11, "400", .88)], "hw")

    s.append(apath("M 340 320 L 458 198", "#cf7714", 2.6, marker="ar"))
    s.append(txt(352, 214, "小消息 · 命令/应答", 11.5, "#cf7714", "start", "700"))
    s.append(apath("M 742 198 L 858 181", "#cf7714", 2.6, marker="ar"))

    s.append(apath("M 340 345 C 400 345, 400 330, 458 330", "#27ae60", 2.6, dash="6 4", marker="arg"))
    s.append(txt(352, 372, "只传 buffer 句柄", 11.5, "#1e8449", "start", "700"))
    s.append(apath("M 742 327 L 858 327", "#27ae60", 2.6, marker="arg"))
    s.append(txt(800, 316, "直接读写", 11, "#1e8449", "middle", "700"))

    s += panel(60, 462, 1080, 120, "性能含义")
    s.append(txt(84, 516, "会话创建（一次性）：几百毫秒 ~ 数秒", 14, "#a93226", weight="700"))
    s.append(txt(84, 546, "包含 algAlloc 分配内存 + algInit 初始化各层核函数", 12, C["muted"]))
    s.append(txt(84, 566, "绝大多数版本错配的失败都发生在这一步", 12, C["muted"]))
    s.append(txt(660, 516, "逐帧推理：几毫秒", 14, "#1e8449", weight="700"))
    s.append(txt(660, 546, "务必复用 session，不要每帧重建", 12, C["muted"]))
    s.append(apath("M 640 500 L 640 566", "#dde3ee", 2, marker=None))
    save("05-ipc", s)


# ══════════════════════ 06 推理时序 ══════════════════════
def d06():
    W, H = 1240, 820
    s = head(W, H, "一次推理的完整时序", "阶段一只做一次（慢），阶段二可重复（快）")

    actors = [("用户程序", "app"), ("ONNX Runtime", "fw"), ("TIDL EP", "backend"),
              ("libvx_tidl_rt", "rt"), ("libtivision_apps", "vx"), ("C7x 固件", "fwc")]
    n = len(actors)
    x0, gap = 105, 190
    xs = [x0 + i * gap for i in range(n)]
    for (name, col), x in zip(actors, xs):
        s += card(x - 80, 92, 160, 46, [(name, 12, "700")], col)
        s.append(apath(f"M {x} 142 L {x} 760", "#d3d9e6", 1.6, marker=None, dash="4 4"))

    # 阶段一
    s.append(f'<rect x="55" y="160" width="{W-110}" height="316" rx="12" fill="#fff6e9" stroke="#f0c896" stroke-width="1.4"/>')
    s.append(txt(74, 186, "阶段一 · 会话创建（只做一次）", 14, "#b9770e", weight="700"))

    steps1 = [(0, 1, "InferenceSession(model, artifacts)"), (1, 2, "查询可 offload 的算子"),
              (2, 2, "读 allowedNode.txt → 划分子图"), (2, 3, "创建 TIDL 子图"),
              (3, 4, "构造 OpenVX 图 + TIDLNode"), (4, 5, "TIVX_CMD_NODE_CREATE")]
    y = 216
    for a, b, label in steps1:
        if a == b:
            s.append(apath(f"M {xs[a]} {y} L {xs[a]+46} {y} L {xs[a]+46} {y+18} L {xs[a]+4} {y+18}", "#5a6480", 1.8))
            s.append(txt(xs[a] + 54, y + 6, label, 11, C["ink"]))
            y += 36
        else:
            s.append(arrow(xs[a], y, xs[b], y, "#5a6480", 1.8))
            s.append(txt((xs[a] + xs[b]) / 2, y - 7, label, 10.5, C["ink"], "middle"))
            y += 30

    # C7x 自身处理（放在生命线上，不越界）
    s += card(xs[5] - 88, y + 4, 176, 58, [("algAlloc  分配内存", 11, "700"),
                                           ("algInit  初始化核函数", 11, "700")], "hw")
    b, bwid = pill(xs[2] - 60, y + 18, "失败点几乎都在这里", "#eb4d4b")
    s += b
    s.append(apath(f"M {xs[2]+bwid-60} {y+30} L {xs[5]-94} {y+30}", "#eb4d4b", 1.8, dash="5 4", marker="arr"))

    s.append(arrow(xs[5], y + 78, xs[4], y + 78, "#27ae60", 1.8, marker="arg"))
    s.append(txt((xs[4] + xs[5]) / 2, y + 70, "ack", 11, "#1e8449", "middle", "700"))

    # 阶段二
    s.append(f'<rect x="55" y="500" width="{W-110}" height="256" rx="12" fill="#eafaf1" stroke="#a9dfbf" stroke-width="1.4"/>')
    s.append(txt(74, 526, "阶段二 · 逐帧推理（可重复，几毫秒）", 14, "#1e8449", weight="700"))

    steps2 = [(0, 1, "run(input)"), (1, 2, "执行子图"), (2, 3, "填输入 buffer（零拷贝）"),
              (3, 4, "vxScheduleGraph"), (4, 5, "TIVX_CMD_NODE_EXECUTE"),
              (5, 4, "完成"), (4, 1, "输出 buffer"), (1, 0, "output")]
    y = 556
    for a, b2, label in steps2:
        col = "#5a6480" if a < b2 else "#27ae60"
        mk = "ar" if a < b2 else "arg"
        s.append(arrow(xs[a], y, xs[b2], y, col, 1.8, marker=mk))
        s.append(txt((xs[a] + xs[b2]) / 2, y - 7, label, 10.5, C["ink"], "middle"))
        y += 25
    save("06-inference-flow", s)


# ══════════════════════ 07 子图切分 ══════════════════════
def d07():
    W, H = 1200, 700
    s = head(W, H, "子图切分：决定性能的核心机制", "子图越少越好 —— 每次进出 C7x 都要同步 + 数据搬运")

    s += panel(60, 100, 480, 300, "原始 ONNX 图")
    nodes = [("Conv", "gray"), ("ReLU", "gray"), ("Conv", "gray"),
             ("自定义算子", "warn"), ("Conv", "gray"), ("Softmax", "gray")]
    y = 142
    for name, col in nodes:
        s += card(180, y, 240, 34, [(name, 12.5, "700")], col, r=8)
        if y < 316:
            s.append(arrow(300, y + 34, 300, y + 41, "#8b93ab", 1.8))
        y += 41
    b, _ = pill(430, 265, "不支持", "#eb4d4b", size=10.5, h=20)
    s += b

    s.append(apath("M 556 250 L 630 250", "#5a6480", 3))
    s.append(txt(593, 238, "TIDL", 12, C["ink"], "middle", "700"))

    s += panel(660, 100, 480, 300, "切分结果")
    parts = [("子图 0  →  C7x", "Conv / ReLU / Conv", "ok", 62),
             ("Node  →  A53 CPU", "自定义算子", "warn", 52),
             ("子图 1  →  C7x", "Conv / Softmax", "ok", 62)]
    y = 146
    for t, sub, col, hh in parts:
        s += card(700, y, 400, hh, [(t, 13.5, "700"), (sub, 11, "400", .88)], col)
        if y < 300:
            s.append(arrow(900, y + hh, 900, y + hh + 10, "#8b93ab", 1.8))
        y += hh + 20

    # 实测表
    s += panel(60, 430, 1080, 190, "Mo 62A 实测", "   同一套编译参数 · int8")
    rows = [("regNetX-200mf", "1", "103 / 103", "5.83 ms", "#1e8449"),
            ("YOLOX-nano-lite", "1", "283 / 283", "9.34 ms", "#1e8449"),
            ("SWIN-tiny（TI 官方）", "4", "594 / 606", "无法运行", "#a93226")]
    cols = [110, 480, 660, 900]
    hdr = ["模型", "子图数", "Offload 节点", "结果"]
    for cx, h in zip(cols, hdr):
        s.append(txt(cx, 486, h, 12.5, C["muted"], weight="700"))
    s.append(apath("M 100 498 L 1100 498", "#dde3ee", 1.4, marker=None))
    y = 528
    for name, sg, off, res, col in rows:
        s.append(txt(cols[0], y, name, 13, C["ink"]))
        s.append(txt(cols[1], y, sg, 13, C["ink"], weight="700"))
        s.append(txt(cols[2], y, off, 13, C["ink"], font=FM))
        s.append(txt(cols[3], y, res, 13, col, weight="700"))
        y += 32

    s.append(txt(100, 604, "运行时这行是第一诊断依据：", 12, C["muted"]))
    s.append(txt(300, 604, "Final number of subgraphs created are : 1, - Offloaded Nodes - 283, Total Nodes - 283",
                 11.5, "#2d3748", font=FM))
    save("07-subgraph", s)


# ══════════════════════ 08 离线编译 ══════════════════════
def d08():
    W, H = 1240, 720
    s = head(W, H, "离线模型编译流程", "模型不能直接部署 —— 必须先在 x86 上编译，设备端不做编译")

    s += panel(60, 100, 720, 430, "x86 主机（Ubuntu）", "   edgeai-tidl-tools")
    s += card(90, 152, 200, 66, [("原始模型", 14.5, "700"), ("model.onnx", 11, "400", .88)], "app")
    s += card(90, 240, 200, 66, [("校准数据", 14.5, "700"), ("N 张代表性图片", 11, "400", .88)], "gold")

    s += card(340, 152, 410, 62, [("TIDLCompilationProvider", 14.5, "700")], "backend")
    s.append(arrow(292, 185, 336, 185, "#8b93ab"))
    s.append(arrow(292, 273, 336, 215, "#8b93ab"))

    steps = [("① 算子适配性分析", "→ 生成 allowedNode.txt", "fw"),
             ("② int8 量化校准", "统计各层激活值范围", "ipc"),
             ("③ 内存规划", "L2 / MSMC / DDR 分配", "sram")]
    y = 238
    for t, sub, col in steps:
        s += card(340, y, 410, 56, [(t, 13.5, "700"), (sub, 10.5, "400", .88)], col)
        if y < 340:
            s.append(arrow(545, y + 56, 545, y + 64, "#8b93ab", 1.8))
        y += 64

    s += card(90, 340, 200, 172, [("artifacts/", 14, "700"), ("", 3),
                                  ("subgraph_0_tidl_net.bin", 9.5, "400", .9),
                                  ("subgraph_0_tidl_io_1.bin", 9.5, "400", .9),
                                  ("allowedNode.txt", 9.5, "400", .9),
                                  ("onnxrtMetaData.txt", 9.5, "400", .9)], "ok")
    s.append(apath("M 336 430 L 300 430", "#27ae60", 2.4, marker="arg"))

    s += panel(830, 100, 350, 430, "Mo 62A 设备")
    s += card(858, 250, 294, 120, [("加载 artifacts", 15, "700"),
                                   ("TIDLExecutionProvider", 11, "400", .88),
                                   ("", 3), ("设备端不做编译", 12.5, "700", .95)], "hw")
    s.append(apath("M 294 486 L 1005 486 L 1005 378", "#27ae60", 2.6, marker="arg"))
    s.append(txt(640, 474, "scp 拷贝 artifacts", 12.5, "#1e8449", "middle", "700"))

    s += warn_line(60, 566, "最坑的一条：calibration_frames 默认 20。喂的帧数不够 → 量化不触发 → 不写最终产物 → 进程仍返回 0，不报任何错。", w=1120)
    s.append(txt(78, 626, "判据：编译日志里", 12.5, C["muted"]))
    s.append(txt(196, 626, 'grep -c "TIDLRT_invoke failed"', 12, "#2d3748", font=FM, weight="700"))
    s.append(txt(420, 626, "必须为 0。", 12.5, C["muted"]))
    s.append(txt(492, 626, "Subgraph Compiled Successfully 不能说明产物可用。", 12.5, "#a93226", weight="700"))
    save("08-compile", s)


# ══════════════════════ 09 GStreamer 流水线 ══════════════════════
def d09():
    W, H = 1280, 620
    s = head(W, H, "GStreamer 全硬件流水线", "整条链数据不出 DMA buffer —— 零拷贝")

    y = 130
    s += card(60, y, 150, 74, [("IMX219", 14, "700"), ("v4l2src", 10.5, "400", .88)], "slate")
    s += card(232, y, 165, 74, [("tiovxisp", 14, "700"), ("VPAC / ISP", 10.5, "400", .88),
                                ("RAW → NV12", 10, "400", .8)], "vpac")
    s.append(arrow(212, y + 37, 228, y + 37, "#8b93ab"))

    # tee
    s.append(f'<circle cx="450" cy="{y+37}" r="26" fill="url(#g_dark)" filter="url(#sh)"/>')
    s.append(txt(450, y + 42, "tee", 13, "#fff", "middle", "700"))
    s.append(arrow(399, y + 37, 422, y + 37, "#8b93ab"))

    # 显示支路
    ydisp = 108
    s += card(520, ydisp, 200, 68, [("tiovxmultiscaler", 13, "700"), ("VPAC / MSC", 10.5, "400", .88)], "vpac")
    s.append(apath(f"M 476 {y+30} C 500 {y+30}, 500 {ydisp+34}, 516 {ydisp+34}", "#8b93ab", 2.2))

    # 推理支路
    yinf = 250
    s += card(520, yinf, 200, 68, [("tiovxdlpreproc", 13, "700"), ("归一化 / 格式", 10.5, "400", .88)], "fw")
    s += card(748, yinf, 190, 68, [("tidlinferer", 13.5, "700"), ("C7x 推理", 10.5, "400", .88)], "hw")
    s += card(966, yinf, 190, 68, [("tidlpostproc", 13, "700"), ("后处理", 10.5, "400", .88)], "backend")
    s.append(apath(f"M 476 {y+44} C 500 {y+44}, 500 {yinf+34}, 516 {yinf+34}", "#8b93ab", 2.2))
    s.append(arrow(722, yinf + 34, 744, yinf + 34, "#8b93ab"))
    s.append(arrow(940, yinf + 34, 962, yinf + 34, "#8b93ab"))

    s += card(1080, ydisp, 140, 68, [("kmssink", 13.5, "700"), ("HDMI 输出", 10.5, "400", .88)], "ok")
    s.append(arrow(722, ydisp + 34, 1076, ydisp + 34, "#8b93ab"))
    s.append(apath(f"M 1160 {yinf+34} C 1210 {yinf+34}, 1240 {yinf}, 1240 {ydisp+90} L 1240 {ydisp+50} L 1224 {ydisp+42}", "#8b93ab", 2.2, marker="ar"))

    # 插件表
    s += panel(60, 360, 1160, 150, "两套插件")
    s.append(txt(84, 400, "tiovx  —— 硬件加速", 13.5, "#e08424", weight="700"))
    s.append(txt(84, 426, "tiovxisp · tiovxmultiscaler · tiovxdlpreproc · tiovxdlcolorconvert · tiovxldc · tiovxmosaic · tiovxmux / demux", 11.5, C["muted"], font=FM))
    s.append(txt(84, 462, "ti  —— 更高层封装", 13.5, "#4834d4", weight="700"))
    s.append(txt(84, 488, "tidlinferer · tidlpreproc · tidlpostproc · tiscaler · timosaic · tiperfoverlay · ticolorconvert", 11.5, C["muted"], font=FM))

    s += warn_line(60, 540, "检测支路不能再放第二个 tiovxmultiscaler（VPAC 争抢 → 18fps 掉到 5fps），需第二路缩放请用软件 videoscale。停流水线一律用 SIGINT，SIGKILL 会让 VPAC 损坏直到重启。", w=1160)
    save("09-gstreamer", s)


# ══════════════════════ 10 版本依赖矩阵 ══════════════════════
def d10():
    W, H = 1240, 800
    s = head(W, H, "版本依赖：五者必须同版本", "错配时的报错极具误导性 —— 四种完全不同的错误其实是同一个病因")

    items = [("① C7x 固件", "am62a-c71_0-fw", "fwc"),
             ("② OpenVX 框架", "libtivision_apps.so", "vx"),
             ("③ TIDL-RT", "libvx_tidl_rt.so", "rt"),
             ("④ 后端库", "libtidl_onnxrt_EP.so", "backend"),
             ("⑤ 模型 artifacts", "subgraph_*_tidl_net.bin", "ok")]
    cw, cgap = 202, 22
    x = (W - (cw * 5 + cgap * 4)) / 2
    for i, (t, sub, col) in enumerate(items):
        s += card(x, 106, cw, 84, [(t, 13.5, "700"), (sub, 9.5, "400", .88)], col)
        if i < len(items) - 1:
            s.append(f'<circle cx="{x+cw+cgap/2}" cy="148" r="9" fill="#fff" stroke="#c7cddb" stroke-width="2"/>')
            s.append(txt(x + cw + cgap / 2, 152, "=", 12, "#6b7280", "middle", "700"))
        x += cw + cgap

    s += card(60, 212, 1120, 44, [("edgeai-tidl-tools（x86 编译工具）必须与上述同一个 REL 版本", 13.5, "700")], "gold")

    # 错误表
    s += panel(60, 282, 1120, 380, "错配时的表现（实测记录）")
    cols = [90, 480, 880]
    for cx, h in zip(cols, ["只换了什么", "报错信息", "真实病因"]):
        s.append(txt(cx, 346, h, 12.5, C["muted"], weight="700"))
    s.append(apath("M 84 360 L 1156 360", "#dde3ee", 1.4, marker=None))

    rows = [("框架版本（ORT 1.23）", "Could not load function from share object file", "EP 库缺符号", "#a93226"),
            ("+ 新 EP 库", "Create state function failed. Return value:-1", "RT / 固件仍是旧的", "#a93226"),
            ("+ 新 RT 库", "同上", "固件仍是旧的", "#a93226"),
            ("+ 新固件", "'config' should be tivxTIDLParms", "tivision_apps 仍是旧的", "#a93226"),
            ("+ 新 tivision_apps", "DSP: TIVX_CMD_NODE_CREATE failed", "artifacts 仍是旧的", "#a93226"),
            ("+ 重编 artifacts", "正常运行", "—", "#1e8449")]
    y = 394
    for a, b2, c2, col in rows:
        if col == "#1e8449":
            s.append(f'<rect x="84" y="{y-19}" width="1072" height="36" rx="8" fill="#eafaf1"/>')
        s.append(txt(cols[0], y, a, 12.5, C["ink"]))
        s.append(txt(cols[1], y, b2, 11, col, font=FM))
        s.append(txt(cols[2], y, c2, 12.5, col, weight="700"))
        y += 42

    s += warn_line(60, 688, "结论：升级 TIDL = 所有已部署模型必须重新编译。旧 artifacts 在新栈上必然失败 —— 这是升级最大的隐性成本。", w=1120)
    s.append(txt(84, 754, "官方下载：", 12.5, C["muted"], weight="700"))
    s.append(txt(168, 754, "software-dl.ti.com/jacinto7/esd/tidl-tools/$REL/FIRMWARES/AM62A/edgeai/11_1/{firmware,tidl_lib}.tar.gz",
                 11.5, "#2d3748", font=FM))
    save("10-version-matrix", s)


if __name__ == "__main__":
    print("生成 SVG:")
    for fn in [d01, d02, d03, d04, d05, d06, d07, d08, d09, d10]:
        fn()
