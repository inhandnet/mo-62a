# HDMI 采集卡 PC 端操作说明

本目录提供 Mo62A 工厂 HDMI 显示/音频回环测试的 PC 端参考实现。

## 测试拓扑

```text
Mo62A HDMI out ──HDMI线──▶ HDMI 采集卡 ──USB线──▶ Windows PC
                                    ▲
                                    │ (scp)
                                    ▼
                           以太网 ───▶ Mo62A
```

PC 端只负责**采集 + 转发**，所有 PASS/FAIL 判定由 Mo62A 端完成。

## 硬件要求

- HDMI 采集卡：支持 HDMI 输入 + USB 输出，带 UAC 音频（Windows 识别为 USB Audio）
- USB 线：采集卡接 PC
- HDMI 线：Mo62A 接采集卡
- 网线：PC 与 Mo62A 同局域网

## 软件要求

| 工具 | 用途 | 说明 |
|---|---|---|
| `ffmpeg.exe` | 抓图、录音 | 下载 Windows build，加到 PATH |
| `scp.exe` | 传文件到 Mo62A | Windows 10/11 自带 OpenSSH |
| Python 3（可选） | 跑示例脚本 | 仅用于 `pc_hdmi_capture.py` |

ffmpeg 下载：https://ffmpeg.org/download.html#build-windows

## 查看采集卡设备名

以管理员身份打开 PowerShell：

```powershell
ffmpeg.exe -list_devices true -f dshow -i dummy
```

记下实际名称，例如：

```text
DirectShow video devices: "USB Video"
DirectShow audio devices: "Digital Audio Interface (USB Audio)"
```

把这两个名字填进 `pc_hdmi_capture.py` 的 `VIDEO_DEV` 和 `AUDIO_DEV`。

## 显示测试流程

### 1. Mo62A 端启动命令

```bash
sudo display hdmi detect
```

- HDMI 显示器显示二维码
- 参考图保存到 `/tmp/mo_hdmi_test.png`
- 命令等待 `/tmp/mo_hdmi_cap.png`（默认 60 秒）

### 2. PC 端抓图并回传

```powershell
ffmpeg.exe -y -hide_banner -loglevel error `
  -f dshow -i video="USB Video" `
  -vframes 1 mo_hdmi_cap.png

scp.exe -o StrictHostKeyChecking=no `
  mo_hdmi_cap.png debian@Mo62A.local:/tmp/mo_hdmi_cap.png
```

### 3. Mo62A 端输出

```text
OK!
```

## 音频测试流程

### 1. Mo62A 端启动命令

```bash
sudo audio test hdmi
```

- HDMI 输出 1kHz 测试音
- 参考音频保存到 `/tmp/mo_audio_test.wav`
- 命令等待 `/tmp/mo_hdmi_audio.wav`（默认 60 秒）

### 2. PC 端录音并回传

```powershell
ffmpeg.exe -y -hide_banner -loglevel error `
  -f dshow -i audio="Digital Audio Interface (USB Audio)" `
  -t 2 -acodec pcm_s16le -ar 48000 -ac 2 `
  mo_hdmi_audio.wav

scp.exe -o StrictHostKeyChecking=no `
  mo_hdmi_audio.wav debian@Mo62A.local:/tmp/mo_hdmi_audio.wav
```

### 3. Mo62A 端输出

```text
OK!
```

## 自动化脚本

```powershell
python pc_hdmi_capture.py --host Mo62A.local
```

参数：

- `--host`：Mo62A 主机名或 IP
- `--video-only`：只测显示
- `--audio-only`：只测音频

## 无采集卡自测

Mo62A 端自带 loopback 模式，不依赖 PC：

```bash
sudo display hdmi detect --loopback TEST123
sudo audio test hdmi --loopback
```

## 常见问题

**Q: scp 提示密码？**  
A: 默认密码 `123456`，已改则替换。

**Q: 设备名含空格？**  
A: ffmpeg dshow 设备名用双引号包裹，如 `video="HDMI Capture"`。

**Q: 超时可以调整吗？**  
A: Mo62A 端支持 `--timeout N`，如 `audio test hdmi --timeout 30`。

**Q: 命令输出只有 OK!/FAIL!，没有诊断？**  
A: 加环境变量 `MO_FACTORY_VERBOSE=1` 可显示 stderr 诊断信息。
