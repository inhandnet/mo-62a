# HDMI 产测（视频二维码 + 音频测频）

## 拓扑与前提
```
Mo62A HDMI ─► HDMI 采集卡(USB) ─► Windows PC ─(SSH 回填)─► Mo62A
```
- Windows 依赖：`pip install pyzbar pillow numpy`；`ffmpeg.exe`（命令用 `--ffmpeg` 指定）。
- 采集卡是 HDMI sink：**PC 打开其流(激活)Mo62A 才输出**；且**只有视频流被采集时才解 HDMI 音频**
  （故抓音频必须视频+音频同开，脚本已内置）。一旦激活过，不重插就保持 connected。

## PC 端 `mo_hdmi_capture.py`（`--ffmpeg` 放子命令前；stdout 只出结果值）

视频三步（配合 `display hdmi detect`）：

| 命令 | 作用 | 出 |
|---|---|---|
| `find` | 获取视频采集卡名 | 如 `UGREEN 25854` |
| `capture <name> [--out cap.png] [--seconds 6]` | 抓一帧(自带激活,采 N 秒取末帧) | png 路径 |
| `decode <png>` | 解析二维码 | 二维码内容 |
| `activate <name> [--seconds 3]` | （可选）纯激活视频流 | OK/FAIL |

音频三步（配合 `audio test hdmi`）：

| 命令 | 作用 | 出 |
|---|---|---|
| `find-audio` | 获取音频采集卡名 | 如 `数字音频接口 (UGREEN 25854)` |
| `capture-audio <name> [--out cap.wav] [--seconds 5] [--video 名]` | 录一段(**视频+音频同开**,不给 --video 自动找视频设备) | wav 路径 |
| `detect-freq <wav> [--min-snr 15]` | FFT 测主频 | stdout=频率；stderr=`freq/snr/sine` |

完整闭环（产测框架编排）：
```
显示: find → SSH:sudo display hdmi detect → capture → decode → SSH 回填字符串 → OK!/FAIL!
音频: find-audio → SSH:sudo audio test hdmi → capture-audio → detect-freq → SSH 回填频率 → OK!/FAIL!
```

## 设备端命令

### `display hdmi detect`（Challenge Code）
1. 随机生成 **16~32 位** Challenge Code：字符集 = 可打印 ASCII `0x21`–`0x7E`
   （大小写+数字+符号，**排除空格/回车换行**），`secrets` 随机；设备不打印明文
   （`MO_FACTORY_VERBOSE=1` 才打到 stderr）。
2. 停 lightdm，等 HDMI 连上（≤15s）。
3. `qrcode`（纠错 M）生成 → 放大 800×800 → 居中画到 `/dev/fb0` 全屏。
4. 打印 `Challenge Code:`（stderr），从 stdin 读一行（默认 30s，`--timeout N`），去尾部 CR/LF。
5. **精确比对** → `OK!`/`FAIL!`（stdout）；恢复 lightdm。
- `--selftest`：不等 HDMI、显示后自动回填自身值（无采集卡自测）。

### `audio test hdmi`（Challenge 频率）
1. 从固定集合 **`[500,800,1100,1500,2000,2600,3200]` Hz** 随机挑一个（`secrets`；间隔大易分辨）。
2. 确认 HDMI 声卡(card1)，等 HDMI 连上（≤15s）。
3. `hw:1,0` 用 gst `audiotestsrc wave=sine` **持续播**该频率正弦（48kHz 立体声，vol 0.8）。
4. 打印 `Tone Freq (Hz):`（stderr），从 stdin 读一行（默认 30s，`--timeout N`），解析为数值。
5. **容差比对** `|回填−生成| ≤ 60Hz` → `OK!`/`FAIL!`；停播音。
- `--selftest`：播放后自动回填生成频率（自测）。

共同：需 root；判定只在 stdout 输 `OK!`/`FAIL!`（退出码 0/非0）；提示/诊断走 stderr；
随机值防脚本假过；`MO_FACTORY_VERBOSE=1` 打印明文答案仅调试用，产线不加。
