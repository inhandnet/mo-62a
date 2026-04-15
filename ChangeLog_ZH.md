# 更新日志

## v1.0.0 — 2026-04-15

MO-62A 板级支持包首次公开发布。

### 内核与设备树

#### HDMI 音频（McASP0 → SiI9022）
- 启用 McASP0 作为 I2S 发送器：AXR2 引脚连接至 SiI9022 SD0，音频时钟
  24.576 MHz，立体声 I2S 格式。
- 启用 `sound-hdmi` simple-audio-card，将 McASP0（CPU DAI）与 SiI9022
  （Codec DAI）关联；CPU 端作为位时钟/帧时钟主机。
- 在声卡节点添加 `playback-only`，防止 PipeWire 在启动时探测不存在的
  录音方向，从而消除 dmesg 中反复出现的 McASP 报错。
- 修复 SiI9022 DRM 桥接驱动中的 `sii902x_bridge_edid_read()`：读取 EDID
  后通过 `drm_detect_hdmi_monitor()` 设置 `sink_is_hdmi` 标志。在
  bridge-chain 模式下，原有的设置路径从未被执行，导致芯片始终工作在
  DVI 模式而无法输出音频。
- 修复 `simple-card-utils.c` 中的 `graph_util_parse_link_direction()`：
  将赋值逻辑改为 OR 逻辑，使声卡根节点上设置的 `playback-only` 不会被后续
  cpu/codec 子节点的检查静默覆盖为 false。

#### 实时时钟（PCF85263A）
- 将 DTS 中的 `compatible` 字符串从 `nxp,pcf8563` 更正为 `nxp,pcf85263`，
  使内核加载正确的 `rtc-pcf85363` 驱动。错误的驱动会误解所有寄存器偏移，
  导致在备用电池电压正常的情况下仍频繁报出 "low voltage detected" 告警。

#### PWM 风扇控制
- 启用 `main_timer7` 作为 PWM 输出（J6 连接器 D18 引脚，TIMER_IO7）。
- 添加 `dmtimer-pwm` 和 `pwm-fan` 设备树节点：25 kHz PWM，四档调速，
  对应 CPU 温度 40–75 °C 区间。
- 在 `k3_j72xx_bandgap` 驱动中通过 `devm_thermal_add_hwmon_sysfs()` 将
  AM62A 热区暴露给 hwmon sysfs，使 `fancontrol` 守护进程可以直接读取
  CPU 温度。

#### IMX219 CSI 摄像头
- 在 I2C2 上添加 IMX219 摄像头节点：XCLK 来自 AM62A CLKOUT0（25 MHz），
  GPIO0_87 作为 XSHUTDOWN，VANA/VDIG/VDDL 通过共享的 `vcc_cam` 稳压器
  供电。
- 针对 25 MHz 输入重新计算 IMX219 PLL 寄存器（PREPLLCK=5，
  PLL_OP_MPY=182 → 链路 455 MHz，PLL_VT_MPY=91 → 像素率 182 Mpix/s）。
- 新增 `csi0_mclk_pins` pinmux 组，用于 CLKOUT0 引脚复用。

#### 40 针扩展接口
- 将所有 40 针接口引脚默认配置为 GPIO 模式（mux=7）。
- 在 DTS 中禁用 `main_uart5`、`main_spi0`、`epwm1` 和 `wkup_i2c0`，
  将对应引脚释放给 GPIO 控制器。
- 根据原理图和实测结果修正 GPIO/EHRPWM 引脚分配：EHRPWM0 A/B 对应
  Pin32/33，EHRPWM1_A 对应 Pin36；修正 GPIO1 和 GPIO0 的 line 编号。

### 根文件系统

- 预装并配置 `fancontrol` 和 `lm-sensors`，开机自动启动 `fancontrol` 服务。
- 禁用 DPMS 和 X11 息屏功能（`xorg.conf.d/10-no-dpms.conf` + `lightdm.conf`），
  防止显示器自动熄屏后无法唤醒。
- 将 `imx219-preview.sh` 安装至 `/usr/local/bin`，支持一键启动 CSI 摄像头
  预览；脚本会自动检测 `/dev/videoX` 和 IMX219 子设备节点。

### 工具

- `mo-62a-flash.sh`：统一烧录工具，支持在线模式（直接写入 `/dev/sdX`）和
  离线模式（生成镜像文件供 balenaEtcher 使用）。
- `setup.sh`：精简的宿主机环境初始化脚本（系统检查、dialout 用户组、
  软件包安装、`~/.bashrc` 中写入 `TI_SDK_PATH`、`/opt` 工具链符号链接）。
- WirePlumber 命名规则：将两个 PipeWire 音频输出的通用名称
  "Built-in Audio Stereo" 分别重命名为 "Headphone Jack (3.5mm)" 和
  "HDMI Audio Output"，便于在音量控制应用中区分。

### 文档

- `README.md` / `README_ZH.md`：完整的板卡上手指南，涵盖代码克隆、工具链
  配置、U-Boot 编译、内核/DTB 编译、烧录、分区布局、40 针 GPIO 使用、
  CSI 摄像头、PWM 风扇、HDMI 音频等内容。
- `doc/QuickStart/`：中英文快速上手指南，包含 balenaEtcher 操作截图、经过
  验证的 40 针 GPIO 对照表、`gpiod` v2.x 命令示例。
- `doc/Schematic/`：MO-62A 硬件原理图 PDF。
- `doc/Chips/PCF85263A.pdf`：PCF85263A RTC 芯片数据手册。
