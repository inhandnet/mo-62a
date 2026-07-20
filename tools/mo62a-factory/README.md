# mo62a-factory — 工厂产测软件包（内部，不随客户镜像发布）

MO-62A 的**工厂产测/定型**命令，打成一个**完全离线**的本地 deb：产测前 `dpkg -i`
安装，产测后 `apt remove` 清理。**故意不放进 `board-support/rootfs-overlay`**（不进客户
镜像，security）。

## 设计约定
- **命令名/用法以工厂固定接口为准**。本包只是"容器"，每条产测命令按工厂给定的接口名
  和调用方式落到 `pkg/usr/local/bin/<工厂接口名>`。（`onie-syseeprom` 只是一版参考实现，
  暂存于 `drafts/`，不是工厂接口名，待定名后再启用。）
- **完全离线**：`dpkg -i` 不联网、不解析依赖。所以 `Depends:` 只能写 **base 镜像已有**
  的包（当前：`python3`、`python3-libgpiod`）。若某命令需要 base 没有的包，把那个 `.deb`
  一并纳入本包（vendor），不能依赖在线源。
- **可干净卸载**：`apt remove mo62a-factory` / `dpkg -r mo62a-factory` 删除本包安装的所有
  文件。运行时写进硬件（如 EEPROM）或产生的日志不回滚（符合预期）。

## 目录结构
```
tools/mo62a-factory/
├── build.sh              # 打包脚本 -> dist/mo62a-factory_<ver>_<arch>.deb
├── README.md
├── drafts/               # 参考实现/草稿，不进包
└── pkg/                  # 就是安装到板子的文件树
    ├── DEBIAN/
    │   └── control       # 包元信息 + Depends（离线可满足）
    └── usr/local/bin/    # 每条工厂产测命令放这里（工厂接口名）
```

## 加一条产测命令
1. 把命令（Python 或已交叉编译好的 arm64 二进制）放到 `pkg/usr/local/bin/<工厂接口名>`。
2. 需要新依赖且 base 没有 → 更新 `pkg/DEBIAN/control` 的 `Depends:`，并把该依赖 `.deb`
   纳入包（离线）。
3. `bash build.sh` 重新打包。

## 产测命令清单

| 命令 | 功能 |
|---|---|
| `factory-model` | EEPROM 设备定型 |
| `com` | 调试串口 TX↔RX 回环 |
| `net` | 以太网千兆 + ping |
| `wlan` | Wi-Fi 2.4/5G 信号扫描 |
| `bt` | 蓝牙控制器 MAC |
| `storage` | SD 卡容量/读/写 |
| `mem` | DDR 物理容量 |
| `rtc` | RTC 电池保持 |
| `usb` | USB Hub + U 盘裸读 |
| `audio` | 3.5mm 耳机环回 / HDMI 音频回环 |
| `led` | 红/绿 LED |
| `key` | S1 电源键 press/release |
| `fan` | PWM 风扇转速 |
| `dio` | 40-pin GPIO 回环 |
| `camera` | IMX219 CSI 摄像头二维码 |
| `display` | HDMI 显示回环 |

### HDMI 显示/音频回环

HDMI 测试需要 PC 端采集卡配合：Mo62A 输出二维码/1kHz 音频到 HDMI，经采集卡到
PC，PC 把 PNG/WAV 通过 scp 回传到 Mo62A 固定路径，Mo62A 自己解码/FFT 判定。

- 视频：`display hdmi detect [expected_qr]`
  - 回传文件：`/tmp/mo_hdmi_cap.png`
  - 参考文件：`/tmp/mo_hdmi_test.png`
  - 自测（无采集卡）：`display hdmi detect --loopback TEST123`
  - 自定义超时：`display hdmi detect --timeout 30`
- 音频：`audio test hdmi`
  - 回传文件：`/tmp/mo_hdmi_audio.wav`
  - 参考文件：`/tmp/mo_audio_test.wav`
  - 自测（无采集卡）：`audio test hdmi --loopback`
  - 自定义超时：`audio test hdmi --timeout 30`

PC 端采集+回传示例及操作说明见 `pc-tools/hdmi-capture-example/`。

## 离线依赖

本包在 `usr/local/share/mo62a-factory/deps/` 中携带了部分 arm64 deb，安装时由
`postinst` 自动 `dpkg -i`：

- `python3-qrcode` — `display` 命令生成二维码
- `python3-pillow` — qrcode 的图像依赖

这些库已包含在 base 镜像中，但为保产线完全离线，仍一并打包。

## 打包 / 安装 / 卸载
```bash
# 开发机（本仓库）打包
bash tools/mo62a-factory/build.sh
# 产出： tools/mo62a-factory/dist/mo62a-factory_<ver>_arm64.deb

# 工厂板子上（离线）
sudo dpkg -i mo62a-factory_<ver>_arm64.deb     # 装，出现各产测命令
# ... 执行产测 ...
sudo apt remove mo62a-factory                  # 或 sudo dpkg -r mo62a-factory，清理
```
