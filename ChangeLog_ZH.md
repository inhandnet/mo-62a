# 更新日志

## v1.0.8 — 2026-07-20

### 首次启动预配置

#### 通过 `sysconfig.txt` 进行系统预配置

- SD 卡的 BOOT（FAT32）分区新增 `sysconfig.txt`，首次启动时自动应用并随后删除。
  支持项：登录用户（用户名 / 密码 / shell）、root 密码、主机名、SSH 公钥、locale、
  时区、Wi-Fi（SSID / 密码 / 国家码），以及有线静态 IP（否则走 DHCP）。
- 首次启动服务（`mo-62a-firstboot-install`）依次安装随包 `.deb`、扩容 root 文件
  系统，再应用 `sysconfig.txt`。

#### 安全：不再内置默认账户

- 镜像不再内置固定的默认用户 / 密码。账户仅由 `sysconfig.txt` 创建，且该文件在
  首启后被删除，卡上不残留任何凭据。

### 网络

#### PC 直连的有线 link-local

- `end0.nmconnection` 将 `ipv4.link-local` 设为数值枚举 `3`。NetworkManager
  keyfile 要求整数形式，字符串 `enabled` 会被当作 `0`，导致直连以太网口的 PC
  拿不到 `169.254.x.x` link-local 地址。
- `bin/mo-62a-flash.sh` 对每个 NetworkManager keyfile 强制 `root:root` / `0600`
  （git 既不跟踪属主也不跟踪 `0600`，须在烧卡时修正）。

### 启动

#### 直接引导默认项（不再弹交互菜单）

- `extlinux.conf` 不再定义 `menu`，U-Boot 直接引导 `default` 项，而不是弹出
  "Enter choice" 交互菜单。这同时避免了 debug UART 收到回环输入时的启动卡死
  （菜单会把回显当成无穷无尽的非法选择反复读取）。启动 overlay 仍用
  `bootcfg switch <label>` 切换。

#### U-Boot autoboot 打断键

- U-Boot autoboot 现在用 `Ctrl+C`（`\x03`）打断，并去掉了 `d`"延时"键。此前的
  `d` / 空格键可能被正常的串口流量（如回环的 debug UART）触发而中止 autoboot。

#### 内核 console 日志级别

- 内核命令行设置 `loglevel=7`，使 debug 级驱动消息（如 Realtek H5 蓝牙 `BT_DBG`）
  不再刷屏 `ttyS2`，同时保留正常内核日志。

### 加密

#### sa2ul：修复未设密钥 transform 拆除时的崩溃

- TI `sa2ul` 驱动对一个从未做过 DMA 映射（`sc_phys == 0`）的上下文调用
  `dma_unmap_single()`，从而对 `phys_to_virt(0)` 做 cache 维护 —— 只要 transform
  分配后在 `setkey` 之前被释放（AF_ALG bind/close、蓝牙 SMP）就会致命缺页。现为
  unmap 加 `if (ctx->sc_phys)` 守卫，并修复 rekey 的 DMA 映射泄漏与映射失败路径
  上的越界释放。

## v1.0.7 — 2026-06-24

### 内核与设备树

#### Docker 运行时支持

- 在 RT 内核 fragment 中启用 BPF、cgroup 控制器和 seccomp，使 Docker 能够以默认安全配置运行：
  - `CONFIG_BPF_SYSCALL=y`、`CONFIG_BPF_JIT=y`、`CONFIG_CGROUP_BPF=y`
  - `CONFIG_MEMCG`、`CONFIG_BLK_CGROUP`、`CONFIG_CGROUP_PIDS`、
    `CONFIG_CGROUP_FREEZER`、`CONFIG_CPUSETS`、`CONFIG_CGROUP_CPUACCT`
  - `CONFIG_SECCOMP=y`、`CONFIG_SECCOMP_FILTER=y`
- 更新 `am62ax_mo_62a_defconfig`：
  - 内建 `CONFIG_OVERLAY_FS=y`，供 Docker `overlay2` 使用。
  - 启用 nftables / masquerade 以支持 Docker 网络。
  - 启用 `CONFIG_CRYPTO_USER_API_SKCIPHER` 与 `CONFIG_CRYPTO_USER_API_AEAD`，
    通过 AF_ALG 暴露内核加密接口。

#### Wi-Fi Monitor 模式

- 在 Realtek `rtl8821cs` SDIO 驱动中启用 `CONFIG_WIFI_MONITOR=y`，使 cfg80211
  注册 `monitor` 接口类型。
- 注释两处 5 GHz 关联 / 接口类型切换时触发的过严 `rtw_warn_on(1)` 断言；
  这些警告非致命，但会污染内核 taint。

### 外部驱动

#### cryptodev Linux 6.12 兼容性

- 将 `cryptodev-module-1.14` 中基于 `register_sysctl()` 的 verbosity 控制改为
  `proc_create()`，消除 Linux 6.12 上的 `sysctl table check failed` 警告。

### 烧录与构建

#### 外部驱动自动编译

- `bin/mo-62a-flash.sh` 现在在制作镜像阶段遍历 `board-support/extra-drivers/*/`，
  用所选内核源码树交叉编译每个外部驱动，并将模块安装到目标 rootfs。
- cryptodev 模块因此随烧卡自动集成。

#### 首次启动 `.deb` 安装钩子

- 扩展现有首次启动服务：在扩容 root 分区之前，自动安装
  `/usr/local/share/mo-62a/prebuilt-deb/` 目录下的所有 `.deb` 包。
  该目录默认置空；如有需要，客户可自行放入 deb 包。

### 根文件系统

#### Docker 默认启用

- 在 `rootfs-overlay` 中添加 `docker.service` 的 systemd enable 软链，使 Docker
  守护进程首次启动即运行。

#### 预装 tcpdump

- 在 Debian base rootfs 中预装 `tcpdump`（及 `libpcap`），使 monitor 模式下可
  直接进行 802.11 空口抓包，无需目标机联网。

### 工厂测试工具

#### Windows 主机支持

- 移植 `tools/mo62a-auto-test/` 以在 Windows PC 上运行（核心框架使用 paramiko，
  跨平台可用）。
- 在 Windows 命令后端下禁用基于 ping 的网络测试。
- 移除旧原型 `tools/mo62a-tester/`。

## v1.0.6 — 2026-06-18

### Edge AI —— 设备端 C/C++ SDK

#### C/C++ Edge AI 开发 SDK（在板上直接编译推理程序）

- 提供完整的设备端 C/C++ Edge AI SDK，客户可**无需交叉编译环境**，直接在板上编译、调试
  自己的 TIDL / C7x 推理程序：
  - 头文件位于 `/usr/include/edgeai/`（edgeai-dl-inferer API、TFLite / ONNX
    Runtime，以及 TI app-utils 头文件）。
  - 静态库位于 `/usr/lib/edgeai/`（`edgeai_dl_inferer` / `pre` / `post` 及预编译
    的 TFLite 全套依赖）。
  - CMake 包位于 `/usr/lib/cmake/EdgeAI/EdgeAIConfig.cmake`，暴露单一
    `EdgeAI::edgeai` 目标。客户工程只需一句 `find_package(EdgeAI)` +
    `target_link_libraries(... EdgeAI::edgeai)` 即可链接整套依赖（TIDL ONNX
    Runtime、tivision\_apps、OpenCV、GStreamer 等），无需手动指定头文件/库路径。
- 新增示例工程于 `/usr/share/edgeai-cpp-examples/`：
  - `hello_inference/`：最小示例，加载模型 + 跑一次推理（无需摄像头）。
  - `app_edgeai/`：完整"摄像头 → 推理 → HDMI"流水线源码。
  - `configs/`（CSI + USB）与 `DEV_GUIDE.md`。
- 已在硬件上验证：`hello_inference` 通过 `find_package(EdgeAI)` 在板上编译，
  推理时模型全部节点均 offload 到 C7x DSP。

#### 统一 `edgeai-demo` 入口（Python + C/C++，CSI + USB）

- 将 `edgeai-demo` 重构为统一入口，使用同一份 YAML 配置同时驱动 **Python** 与
  **C/C++** 两个后端、**CSI** 与 **USB** 两种摄像头，支持交互与命令行两种模式：
  - `edgeai-demo run <模型> --backend python|cpp --camera csi|usb`
  - 交互模式依次选择 摄像头 → 模型 → 后端。
- USB 摄像头支持：自动选择真正的采集节点（跳过 UVC 仅 metadata 的节点），并按摄像头
  实际支持的像素格式选择（MJPG → jpeg，否则 YUYV），修复了 USB 输入时 GStreamer
  管线构建失败的问题。

#### C/C++ `app_edgeai` —— 显示标题

- 将 `tiperfoverlay` 的 `main-title` 改为可由 YAML 配置并默认关闭，去除硬编码的
  "Texas Instruments Edge AI" 横幅，使 C/C++ demo 的叠加显示与 Python demo 一致。

### 根文件系统

#### 预装 C/C++ 开发包

- 在 base 镜像中预装 `libyaml-cpp-dev`、`libopencv-dev`、
  `libgstreamer-plugins-base1.0-dev`，使 Edge AI SDK 与客户程序开箱即可在板上编译。

#### 修复 `libdrm` 依赖冲突

- 将 `libdrm` 统一为 Debian 官方 `2.4.124-2`（替换非标准的 `2.4.127` 版本），使安装
  `*-dev` 开发包（如 GStreamer plugins-base dev）时不再触发 held / broken 依赖冲突。

### 烧录与构建

- `bin/mo-62a-flash.sh` 现在会在制作镜像时通过 qemu-aarch64 chroot 编译
  `edgeai-cpp`，并将完整 C/C++ SDK（二进制、头文件、库、CMake 包、示例）安装到目标
  rootfs；同时把所选 rootfs tarball 传入，保证编译与运行时 ABI 一致。
- 移除 rootfs overlay 中过时的重复 `apps_cpp` 源码副本；权威的 C/C++ demo 源码现在
  仅随 SDK 示例提供。

## v1.0.5 — 2026-05-07

### 内核与设备树

#### 40-Pin 扩展接口 — 外设模式 DT Overlay

- 新增 `k3-am62a7-mo-62a-exp-periph.dtso`，为 40-pin 扩展接口（J9）提供可选外设功能：
  - **引脚 3/5**：WKUP\_I2C0 SDA/SCL（`/dev/i2c-0`）
  - **引脚 8/10**：UART5 TXD/RXD（`/dev/ttyS3`）
  - **引脚 19/21/23/24/26**：SPI0 D0/D1/CLK/CS0/CS1
  - **引脚 32/33**：EHRPWM0\_B/A（`/sys/class/pwm/pwmchip0` ch1/ch0）
  - **引脚 12/35/38/40**：MCASP2 ACLKX/AFSX/AXR0/AXR1
- Overlay 为 `mcu_gpio0` 和 `main_gpio1` 提供精简的 pinctrl 组（排除已被外设占用的
  引脚），防止 GPIO 控制器在启动时 probe 失败。
- 在 overlay 中将 pad 0x0174（GPIO0\_91 / 引脚 35）从 `gpio0-default-pins` 中移除，
  解决 SiI9022 HDMI 桥接芯片与 MCASP2\_AFSX 之间的 pinctrl 冲突。
- 在内核 DTS `Makefile` 中新增 `dtb-$(CONFIG_ARCH_K3) += k3-am62a7-mo-62a-exp-periph.dtbo`，
  执行 `make linux-dtbs` 时自动编译该 overlay。

#### 40-Pin SPI0 — spidev 节点

- 在 overlay 的 `main_spi0` 下新增 `spidev@0`（CS0，引脚 24）和 `spidev@1`（CS1，引脚 26）
  子节点，`compatible = "rohm,dh2228fv"`（内核新版本拒绝 `"spidev"` 字符串）。
- 节点生效后 `/dev/spidev0.0` 可用，已通过引脚 19（MOSI）↔ 引脚 21（MISO）短接
  硬件环回测试，8 字节 `xfer2` 收发一致，验证通过。

#### 40-Pin 音频 — Waveshare WM8960 Audio HAT 支持

- 在 overlay 的 `wkup_i2c0` 下新增 `wm8960@1a`（`compatible = "wlf,wm8960"`），
  WM8960 驱动通过 WKUP\_I2C0（引脚 3/5，i2c-0）探测成功。
- 完善 `mcasp2` 节点配置：`op-mode = IIS`、`tdm-slots = 2`、`serial-dir`
  （AXR0 = RX 麦克风，AXR1 = TX 耳机）、`system-clock-frequency = 24576000`
  （告知驱动使用 24.576 MHz 计算 BCLK 分频，与 HDMI 音频采用相同方法）。
- 通过 `&{/}` fragment 在根节点新增 `sound-wm8960 simple-audio-card`，
  MCASP2 为 I2S master，WM8960 为 codec slave。
- 验证结果：
  - WM8960 驱动以 0x1a 探测成功（`i2cdetect` 显示 `UU`）
  - ALSA 声卡 `WM8960-Sound` 注册，48 kHz 立体声播放正常
  - 5 秒录音（S32\_LE，板载双 MEMS 麦克风）回放验证通过

#### 40-Pin 扩展接口 — 单独外设 DT Overlay 文件

- 新增 5 个独立外设 overlay 文件，每个文件仅启用一组外设：
  - `k3-am62a7-mo-62a-exp-i2c0.dtso`：仅启用 WKUP\_I2C0（引脚 3/5）；
    MCU GPIO0 pinctrl 精简，排除 `MCU_GPIO0_19/20`。
  - `k3-am62a7-mo-62a-exp-uart5.dtso`：仅启用 UART5（引脚 8/10）；
    GPIO1 pinctrl 精简，排除 `GPIO1_24/25`。
  - `k3-am62a7-mo-62a-exp-spi0.dtso`：仅启用 SPI0（引脚 19/21/23/24/26），
    含两个 spidev 节点；GPIO1 pinctrl 精简，排除 `GPIO1_15–19`。
  - `k3-am62a7-mo-62a-exp-ehrpwm0.dtso`：仅启用 EHRPWM0（引脚 32/33）；
    GPIO1 pinctrl 精简，排除 `GPIO1_13/14`。
  - `k3-am62a7-mo-62a-exp-audio.dtso`：WM8960 Audio HAT——将 WKUP\_I2C0
    （Codec I²C 控制，引脚 3/5）与 MCASP2（I²S 音频数据，引脚 12/35/38/40）
    组合使用；MCU GPIO0、GPIO1 及 `gpio0-default-pins` 的 pinctrl 组均单独
    更新，仅排除本 overlay 所占用的引脚。
- 每个 overlay 携带精确裁剪的 GPIO pinctrl 组，仅移除该外设占用的引脚，
  无论加载哪个单独 overlay，均不会导致 GPIO 控制器 probe 失败。
- 在内核 DTS `Makefile` 中新增对应的 5 条
  `dtb-$(CONFIG_ARCH_K3) += k3-am62a7-mo-62a-exp-<name>.dtbo` 构建目标。

### U-Boot

#### LPDDR4 双芯片兼容 — 三星 2GB / 镁光 4GB 运行时自动检测

- 将 R5 DDR 参数文件从 `lp4-4GB.dtsi` 重命名为 `lp4-Samsung-2GB-1866MHz.dtsi`，
  并对齐经过验证的三星 LPDDR4 时序参数；更新 `k3-am62a7-r5-mo-62a.dts`，以重命名
  后的文件作为默认 DDR 配置。
- 新增 `lp4_micron_4gb.h`，包含镁光 4GB 完整 CTL/PI/PHY 寄存器表（2805 个条目），
  供运行时 DDR 重新初始化使用。
- 将 A53 侧 `k3-am62a7-mo-62a.dts` 中的内存节点更新为 2GB（三星默认值），实际
  大小由运行时 FDT fixup 动态修正。
- 从 `am62ax_mo_62a_r5_defconfig` 中删除未使用的 SPI/NAND/I2C Kconfig 符号。
- 在 `k3-ddrss.c`（R5 SPL / tiboot3.bin）中实现运行时厂商检测：
  - 三星 2GB 初始训练完成后，通过 Cadence DDR 驱动的 `getmmrregister` 接口
    读取 LPDDR4 MR5（厂商 ID）。
  - **MR5 = 0x01（三星）**：不做任何额外操作，保持单 bank 2GB 配置。
  - **MR5 = 0xFF（镁光）**：通过直接写 PSC `MDCTL` 寄存器执行 DDR 子系统复位
    （TI-SCI 在 SPL 阶段无法对共享时钟域断电重启），再以镁光 4GB CTL/PI/PHY 寄存器
    表重新初始化，并设置 `ddr_bank1_size = 0x80000000`，基地址 `0x880000000`。
- 扩展 `board/ti/am62ax/evm.c` 中的 `spl_perform_fixups()`：对于
  `CONFIG_K3_DDRSS` 且未启用 inline ECC 的构建，调用
  `k3_ddrss_fdt_fixup_memory()`，将实际内存布局通过 FDT 链逐级传递：
  - R5 SPL 用真实 bank 布局修正 tispl FDT
  - A53 SPL 读取更新后的 FDT 并修正 U-Boot FDT
  - A53 U-Boot 显示 `DRAM:  2 GiB (total 4 GiB)` 并修正 Linux DTB
  - Linux 内核通过 `/memory` 节点看到完整 4GB
- 三星 2GB 板完全不受影响：MR5 = 0x01 不匹配任何兼容性入口，`bank1_size` 保持
  为零，FDT fixup 仅写入单 bank 2GB；同一张 SD 卡镜像可在两种硬件上正确启动。
- 实测结果：
  - 三星 2GB：`DRAM:  2 GiB`，Linux `/proc/meminfo` 显示约 2GB
  - 镁光 4GB：`DRAM:  2 GiB (total 4 GiB)`，Linux `/proc/meminfo` 显示约 4GB

### 启动配置

- `bin/extlinux/extlinux.conf` 重构为 7 个启动项：
  - `microSD` — 全 GPIO 模式（**默认**）
  - `microSD-i2c0` — WKUP\_I2C0（引脚 3/5）
  - `microSD-uart5` — UART5（引脚 8/10）
  - `microSD-spi0` — SPI0，含两路 spidev CS（引脚 19/21/23/24/26）
  - `microSD-ehrpwm0` — EHRPWM0 PWM 输出（引脚 32/33）
  - `microSD-audio` — WM8960 Audio HAT（WKUP\_I2C0 + MCASP2，引脚 3/5/12/35/38/40）
  - `microSD-periph` — 所有特殊功能同时启用
- **默认启动项为 `microSD`（全 GPIO 模式）。** 如需启用某种外设模式，
  可修改 extlinux.conf 中的 `default` 行，或在启动时中断 U-Boot 并交互式
  选择所需启动项。

---

## v1.0.4 — 2026-04-28

### 内核与设备树

#### SiI9022A HDMI 桥接芯片 — 电源轨注册（k3-am62a7-mo-62a.dts）
- 新增 `vdd_1v2_hdmi: regulator-7` 固定稳压器节点（1.2 V，常开），对应为
  SiI9022ACNU CVCC12 供电的 TLV75512PDQN（U8）LDO。
- 在 `sii9022` 桥接节点中添加 `iovcc-supply = <&vcc_3v3_sys>` 和
  `cvcc12-supply = <&vdd_1v2_hdmi>`，使驱动能够正确找到两条电源轨。
  此前驱动在每次启动时均会打印
  `supply iovcc not found, using dummy regulator` 及
  `supply cvcc12 not found, using dummy regulator` 告警，修复后不再出现。

#### omap-mailbox — 禁用未使用的 Cluster 3（k3-am62a7-mo-62a.dts）
- 在 DTS 中将 `mailbox0_cluster3` 设置为 `status = "disabled"`。
  AM62A7 共有四个 mailbox 硬件实例，但仅有三个 remoteproc 消费者
  （C7x DSP + MCU-R5F + MAIN-R5F），Cluster 3 无任何已注册的 mbox 设备，
  导致内核在每次启动时打印 `omap mailbox: no available mbox devices found`。

#### 设备树依赖循环 — 降低日志级别（drivers/base/core.c）
- 将 `Fixed dependency cycle(s) with` 消息从 `pr_info` 改为 `pr_debug`。
  内核会自动解决这些循环依赖，相关消息仅为信息提示，修改后在默认控制台日志
  级别下不再显示。

#### S1 电源键 — TPS6593-Q1 PMIC 驱动（tps6594-core.c）
- 当 PMIC 设备树节点设有 `system-power-controller` 属性时，在 probe 阶段将
  `NPWRON_CONF` 寄存器（地址 0x3C）的 `NPWRON_SEL` 位域（[7:6]）配置为按键模式
  （`01`）。在此模式下，NPWRON 引脚在 S1 按下/松开时产生中断，而非充当简单的
  使能信号。
- 为 `TPS6594_IRQ_NPWRON_START` 注册 IRQ 处理函数，将 `KEY_POWER` 按下与松开
  事件上报至新建的 `tps6594-pwrbutton` 输入设备。由于 PMIC 每次按压仅产生一次
  上升沿中断，松开事件通过每 50 ms 轮询 `GPIO_IN_2`（NPWRON 输入状态位）的
  `delayed_work` 检测。
- 从 `tps6594_pfsm_resources[]` 中移除 `TPS6594_IRQ_NPWRON_START`，避免与新
  注册的处理函数产生 IRQ 所有权冲突（`-EBUSY`）。
- 新增 `register_reboot_notifier` 回调：在 `SYS_POWER_OFF` 事件时将
  `NPWRON_SEL` 切回使能模式（`00`）。该回调在 `kernel_shutdown_prepare()` 阶段
  执行，此时 I²C 仍正常工作；切换完成后，S1 短按即可在软关机后重新上电启动系统。

### 根文件系统

#### Wi-Fi 信道规范库 — 切换为 upstream 签名版本
- 从 rootfs 压缩包中删除 Debian 签名的 `regulatory.db` 及其独立签名文件
  （`/lib/firmware/regulatory.db-debian` 和 `regulatory.db.p7s-debian`）。
  内核仅内置了 `sforshee` 和 `wens` 的 X.509 证书；Debian 签名版本无法通过
  这些证书验证，导致启动时打印
  `cfg80211: loaded regulatory.db is malformed or signature is missing/invalid`。
- `update-alternatives` 自动模式下，upstream 签名版本
  （符号链接 `/lib/firmware/regulatory.db → regulatory.db-upstream`）被选为
  活动数据库，内核可正常验证，告警消除。
- 后续通过 `apt upgrade` 更新 `wireless-regdb` 包时，`regulatory.db-upstream`
  会就地更新；自动模式仍选择 upstream 版本，内核验证继续正常工作。

#### S1 电源键计时守护进程
- 新增 `board-support/extra-applications/s1-powerkey/` Python 守护进程，
  行为与标准 Ubuntu 笔记本电源键一致：
  - **按下** → 立即弹出 `xfce4-session-logout` XFCE 关机对话框（通过守护线程
    非阻塞触发，无需松开按键）
  - **5 秒内松开** → 取消关机计时器；对话框保持显示，等待用户操作
  - **持续按住 ≥ 5 秒** → `systemctl poweroff`（绕过对话框，强制关机）
  - 无 XFCE 会话（停留在登录界面）时：弹窗步骤静默跳过，5 秒关机计时器仍正常触发
- 新增 `board-support/rootfs-overlay/etc/systemd/system/s1-powerkey.service`：
  简单服务，设置 `Restart=always`；仅 `WantedBy=multi-user.target`，去除
  `After=graphical.target` / `Wants=graphical.target` 依赖——原有依赖会在
  systemd 解析启动顺序时形成循环依赖，导致服务开机静默跳过不启动。
- 新增 `multi-user.target.wants/s1-powerkey.service` 符号链接，实现开机自启。
- 新增 `board-support/rootfs-overlay/etc/systemd/logind.conf.d/s1-powerkey.conf`：
  设置 `HandlePowerKey=ignore` 和 `HandlePowerKeyLongPress=ignore`，阻止
  logind 在守护进程之前消费 `KEY_POWER` 事件。

---

## v1.0.3 — 2026-04-21

### 根文件系统

#### 镜像体积精简（未压缩约 1.76 GB / 压缩后约 400 MB）
- 通过 `apt-get remove --purge` + autoremove 卸载 Thunderbird 和 LibreOffice
  （含所有 l10n 及 UI 软件包），释放约 487 MB。
- 精简 Noto 字体：删除 `fonts-noto-extra`、`fonts-noto-cjk-extra`、
  `fonts-noto-ui-extra`、`fonts-noto-unhinted` 及 `fonts-noto` 元包；
  保留 `fonts-noto-core`、`fonts-noto-cjk`、`fonts-noto-mono`；释放约 657 MB。
- 删除 MO-62A 硬件不使用的第三方厂商固件：Qualcomm（ath10k/ath11k/ath12k/qca）、
  Intel（iwlwifi/intel）、MediaTek、Broadcom（brcm）、
  Marvell（mrvl/libertas/mwl8k）、Cypress、Atheros（ath6k/ar3k）、
  Wilocity、Ralink 及零散固件文件；保留 TI（ti-connectivity/ti-ipc）、
  Realtek（rtw89/rtw88/rtlwifi/rtl_bt/rtlbt/rtl_nic/realtek）、
  AM62A VPU（cnm/vpu_d.bin/vpu_p.bin）及无线信道规范库；释放约 306 MB。
- 精简 locale 数据：仅保留 zh_CN、zh_TW、zh_HK、en、en_US、en_GB 及
  locale.alias，其余全部删除；释放约 307 MB。
- 预装 `memtester` 和 `mbw`，用于 LPDDR4 完整性和带宽测试。

### 工具

#### mo62a-tester — 存储测试套件
- 新增 `tests/test_storage.py`，包含全新的「Storage（存储）」测试类别（`cat_storage`）：
  - `LpddrMemtesterTest`：通过 sudo 执行 `memtester 32M 1`（19 种测试模式）；
    测试大小从 128 M 缩减至 32 M，将单次运行时间控制在 60 秒以内（AM62A 典型约 52 秒）。
  - `LpddrBandwidthTest`：执行 `mbw -n 3 256`，解析 MEMCPY 均值，
    报告 MiB/s，低于 1000 MiB/s 时判定失败（AM62A 典型约 1478 MiB/s）。
  - `SdSpeedModeTest`：通过 sudo 读取 `/sys/kernel/debug/mmc1/ios`，
    获取 SD 卡协商速率模式及时钟频率；失败时回退到 `journalctl` 关键字匹配。
  - `SdReadSpeedTest`：清除页缓存后，使用
    `dd if=/dev/mmcblk1 of=/dev/null bs=4M count=50` 测量顺序读取速度；
    阈值 ≥ 15 MB/s。
  - `SdWriteSpeedTest`：使用
    `dd if=/dev/zero of=/tmp/sd_write_test bs=4M count=50 oflag=dsync`
    测量顺序写入速度；阈值 ≥ 5 MB/s。
- 在 `gui/page_select.py` 的 `TEST_CATEGORIES` 中注册 `cat_storage`。
- 在 `gui/i18n.py` 中添加存储类别相关字符串（中英文）。

---

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

#### 引脚复用无效代码清理
- 从 `k3-am62a7-mo-62a-pinmux.dtsi` 中删除孤立的 `main_ehrpwm1_pins_default`
  引脚组。该组声明了 pad 0x019c（`MCASP0_AXR1`，球 B18）的 EHRPWM1_A 复用，
  但从未被任何 DTS 节点引用，与同样声明 pad 0x019c（GPIO 复用模式）的
  `gpio1_pins_default` 存在潜在冲突。删除该无效条目以消除冲突隐患。
- 修正 `gpio1_pins_default` 中 GPIO1_9（pad 0x019c）的注释，更正信号名称为
  `/* (B18) MCASP0_AXR1.GPIO1_9 */`。

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

#### nginx — 日志目录缺失修复
- 在 rootfs overlay 中新增 `usr/lib/tmpfiles.d/nginx.conf`：指示
  `systemd-tmpfiles` 在启动时创建 `/var/log/nginx/` 目录（属主
  `www-data:adm`，权限 0755），修复基础 rootfs 镜像中该目录缺失
  导致 nginx 服务启动失败的问题。

### 工具

#### mo-version
- 新增 `board-support/extra-applications/mo-version/`：一个轻量 C 工具，
  安装至 `/usr/local/bin/mo-version`，输出在编译时写入的 BSP 版本号和构建日期：
  ```
  MO-62A v1.0.2
  Built:  2026-04-17
  ```
  烧录脚本在调用 make 时传入 `VERSION` 和 `BUILD_DATE` 变量，确保二进制文件
  始终与烧录的镜像版本一致。

### 文档

#### QuickStart — PWM 引脚表格勘误
- 删除 §8.4 中引用 `pwmchip0` 通道 0 控制扩展排针 Pin 32（BCM GPIO12）的
  错误 PWM sysfs 示例。`pwmchip0` 是风扇 PWM 控制器，并非扩展排针；
  该示例会静默地影响风扇而非扩展排针。目前扩展排针无 PWM 输出路由，
  故不提供替代示例。

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
