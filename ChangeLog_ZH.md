# 更新日志

## v1.0.2 — 2026-04-17

### 内核与设备树

#### HDMI 息屏唤醒修复（SiI9022A）
- 修复 `sii902x_bridge_atomic_enable()`：将 20 ms TMDS PLL 稳定延迟
  （`msleep(20)`）移至条件块外，确保在清除 `PWR_DWN` 之前无条件执行。
  原实现将其置于 `if (mode.clock)` 分支内——模块热加载后 `mode.clock`
  为 0（DRM 仅更新 `active_changed`，不重新调用 `mode_set`），导致 PLL
  无法锁定，DPMS 唤醒后画面持续黑屏。
- 在 `atomic_enable` 中添加 CRTC 状态回退逻辑：若 `mode.clock` 仍为 0
  （如未重启的热加载场景），从 `bridge->encoder->crtc->state` 读取
  调整后的显示模式，以便仍能正确编程 TPI 视频寄存器。
- 将 TPI 视频寄存器编程逻辑重构为独立的 `sii902x_apply_mode()` 辅助函数；
  在 `struct sii902x` 中缓存调整后的显示模式，使其在断电和 DPMS 循环中
  无需重新调用 `mode_set` 即可保持有效。
- 在 `sii9022` DTS 节点添加 `reset-gpios = <&main_gpio1 3 GPIO_ACTIVE_LOW>`
  及专用的 `sii9022_reset_pins` 引脚复用组（将 `RGMII2_RD0 / GPIO1_3` 配置为
  `PIN_OUTPUT`），允许驱动在 probe/remove 时主动控制 HDMI_RSTn。

#### EEPROM BL24C02F 驱动支持
- 在 `k3-am62a7-mo-62a.dts` 的 `&main_i2c1` 下新增 `eeprom@50` I2C 设备节点：
  `compatible = "atmel,24c02"`，地址 0x50，页大小 16 字节，
  `wp-gpios = <&main_gpio1 7 GPIO_ACTIVE_HIGH>`。`at24` 驱动
  （`CONFIG_EEPROM_AT24=m`）在启动时由 udev 自动加载，将 EEPROM 以
  `/sys/bus/i2c/devices/1-0050/eeprom`（256 字节，仅 root 可读写）的形式暴露给用户空间。
- 将 `gpio1_pins_default` 中 pad 0x0194（`MCASP0_AXR3`，球 C19）的配置从
  `PIN_INPUT` 改为 `PIN_OUTPUT`。EEP_WC（写控制）信号经 R267（10 kΩ）上拉至
  VCC_3V3_SYS；配置为输入时该引脚浮至高电平，使 EEPROM 处于写保护状态。
  改为输出后，`at24` 驱动可通过 `wp-gpios` 将其驱动为低电平（允许写入）。

#### TIDSS DPMS 唤醒黑屏修复（tidss_plane.c）
- 修复 `drivers/gpu/drm/tidss/tidss_plane.c` 中的 `tidss_plane_atomic_update()`：
  在调用 `dispc_plane_setup()` 后，对可见 plane 额外调用
  `dispc_plane_enable(true)`。
  **根本原因**：DPMS Off 后，`tidss_runtime_put()` 将 PM 引用计数降至 0；
  1 秒自动挂起延迟到期后，`dispc_runtime_suspend()` 关闭 DSS 功能时钟，
  致使硬件掉电。DPMS On 时，`dispc_runtime_resume()` 调用
  `dispc_initial_config()` → `dispc_k3_plane_init()`，将
  `DISPC_VID_ATTRIBUTES` bit 0（VID pipeline 使能位）复位为 0。
  DRM 提交随后调用 `tidss_plane_atomic_update()`（写入 DMA shadow 寄存器），
  但因 `drm_atomic_plane_enabling()` 返回 false（DRM 状态显示 plane 仍绑定
  在 CRTC 上，框架不认为需要重新使能），跳过了
  `tidss_plane_atomic_enable()`。VID pipeline 保持 disabled，overlay 层收不
  到像素数据，显示全黑；与此同时 SiI9022A 仍正常输出 HDMI 同步信号（显示器
  指示灯为白色）。修复方案：在 `atomic_update()` 中只要 plane 可见就无条件
  重新使能 VID pipeline，该调用对正常页翻转是幂等的，对 DPMS 唤醒的硬件掉
  电场景则是必要的。

### 根文件系统

#### DPMS 配置与唤醒守护进程
- 在 rootfs overlay 中添加 `/etc/xdg/autostart/enable-dpms.desktop`：每次
  XFCE 会话启动时执行 `xset +dpms; xset dpms 0 0 600; xset s off;
  xset s noblank; dpms-wakeup &`。启用 DPMS 并设置 10 分钟息屏超时
  （Standby/Suspend 禁用），禁用 X 屏幕保护程序，并启动 `dpms-wakeup` 守护进程。
- 新增 `dpms-wakeup` Python 守护进程（`/usr/local/bin/dpms-wakeup`）：使用
  `select()` 监控所有 `/dev/input/event*` 节点，在 DPMS 息屏状态下检测到键盘或
  鼠标活动时调用 `xset dpms force on`。内置 2 秒冷却时间和事件排空循环，
  防止键盘自动重复引发大量唤醒调用。解决了 Xorg 的限制：在 DPMS 已处于
  Off 状态时，DPMS 空闲计时器不会因物理输入自动唤醒显示器。

#### DPMS 唤醒可靠性修复 — xfce4-power-manager 竞态问题
- 在 rootfs overlay 中添加 `/etc/xdg/autostart/xfce4-power-manager.desktop`
  （`Hidden=true`），全面屏蔽 xfce4-power-manager 在 XFCE 会话中的自动启动。
  **根本原因**：xfce4-power-manager 4.20.0 以固定间隔轮询 XScreenSaver 的空闲
  计时器。当显示器从 DPMS 息屏状态被唤醒（按下键盘或执行 `xset dpms force on`）
  时，XSS 空闲计数器的复位与 xfce4-power-manager 下一次轮询之间存在竞态：若
  轮询在复位完成之前触发，xfce4-power-manager 读取到的空闲时长仍超过
  `dpms-on-ac-sleep` 阈值（4 分钟），随即调用 `DPMSForceLevel(Off)`，导致屏幕
  在唤醒约 1 秒后再次熄灭（表现为屏幕闪烁一下后立刻变黑）。该版本中
  `presentation-mode=true` 配置项对此竞态无效。屏蔽 xfce4-power-manager 后，
  DPMS 完全由 X 服务器接管。

#### USB 输入设备 seat 分配
- 在 rootfs overlay 中添加 `/etc/udev/rules.d/72-seat-input.rules`。
  在 AM62A + LightDM 的组合下，udev 不会自动为 USB 键盘/鼠标/触摸屏/摇杆
  添加 `ID_SEAT=seat0` 标签，原因是 USB Hub 与 DSS/DRM 子系统挂在不同的父设备
  下——logind 因此将这些输入设备排除在 seat0 的设备列表之外，libinput 也无法
  通过 logind 的 TakeDevice 接口向 Xorg 传递物理按键/鼠标事件。没有物理输入
  事件，显示器就无法被键盘或鼠标从 DPMS 息屏状态唤醒。新规则对所有已识别的
  输入设备类型（`ID_INPUT_KEYBOARD`、`ID_INPUT_MOUSE`、`ID_INPUT_TOUCHSCREEN`、
  `ID_INPUT_JOYSTICK`）显式设置 `ID_SEAT=seat0` 并添加 `TAG+="seat"`。

---

## v1.0.1 — 2026-04-15

### 内核与设备树

#### 双色 LED
- 删除原理图中不存在的蓝色 LED 节点（`MCU_GPIO0_2`）——原理图仅包含红色
  （`MCU_GPIO0_16` / PWR_LED）和绿色（`MCU_GPIO0_15` / ACT_LED）两个 LED。
- 修复 LED 引脚复用：将两个 LED 引脚的配置从 `PIN_INPUT` 改为 `PIN_OUTPUT`。
- 修复 LED 极性：将 `GPIO_ACTIVE_HIGH` 改为 `GPIO_ACTIVE_LOW`，以匹配
  三极管驱动的低电平有效电路（GPIO 输出低电平 = LED 点亮）。
- 将红色 LED 的 `default-state` 设为 `"on"`，使其在内核 gpio-leds 初始化时
  立即点亮，清晰指示系统正在启动。

### 根文件系统

#### 双色 LED 状态控制器
- 新增 `led-status` Python 服务（`/usr/local/bin/led-status` +
  `led-status.service`）：在 `multi-user.target` 到达前保持红色 LED 常亮；
  系统启动完成后关闭红色 LED，并将绿色 LED 切换为呼吸灯模式。
  呼吸频率与四核平均 CPU 使用率正相关——0 % 时半周期约 2 000 ms（极慢），
  100 % 时半周期约 100 ms（快速）。

#### nginx — 日志目录缺失修复
- 在 rootfs overlay 中新增 `usr/lib/tmpfiles.d/nginx.conf`：指示
  `systemd-tmpfiles` 在启动时创建 `/var/log/nginx/` 目录（属主
  `www-data:adm`，权限 0755），修复基础 rootfs 镜像中该目录缺失
  导致 nginx 服务启动失败的问题。

#### fancontrol — hwmon 编号漂移修复
- 新增 `fancontrol-update-config` 脚本（`/usr/local/bin/`）：每次服务启动时
  扫描 `/sys/class/hwmon/hwmon*/name`，动态定位 `pwmfan` 和 `main0_thermal`
  的当前 hwmon 编号，并重新生成 `/etc/fancontrol`，从根本上解决重启后
  hwmon 编号变化导致 fancontrol 启动失败的问题。
- 新增 `fancontrol.service.d/override.conf` drop-in 配置：在上游
  `fancontrol --check` 步骤之前先执行 `fancontrol-update-config`，并添加
  `ReadWritePaths=/etc/fancontrol`，允许在 `ProtectSystem=strict` 保护下
  写入配置文件。

---

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
- 将 `imx219-preview.sh` 安装至 `/usr/local/bin`，支持一键启动 CSI 摄像头
  预览；脚本会自动检测 `/dev/videoX` 和 IMX219 子设备节点。

### 工具

- `mo-62a-flash.sh`：统一烧录工具，支持在线模式（直接写入 `/dev/sdX`）和
  离线模式（生成镜像文件供 balenaEtcher 使用）。离线镜像命名规则为
  `mo-62a-<os><ver>-<desktop>-<version>-<date>.img.zip`，
  例如 `mo-62a-debian13.3-xfce-v1.0.0-2026-04-15.img.zip`。
  交互式提示仅询问版本号和日期，其余字段固定不变。
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
