# MO-62A 工厂产测 / 定型接口

> 本文档为 MO-62A 生产测试的**统一接口基准**。新产品的产测接口以此文档切分支；
> 新增测试项须同步回本文档。

## 策略

1. 工厂产测接口（命令）**直接在设备端运行**；
2. 出于 security 考虑，**产测软件不提供给客户**；
3. 所有产测软件打成一个**完全离线**的本地包 `mo62a-factory.deb`（含所需依赖）；
4. 产测前 `dpkg -i mo62a-factory.deb` 安装，安装后出现多个 C / Python 命令；
5. 产测后 `apt remove mo62a-factory` 清理已安装的全部文件。

源码与打包见 `tools/mo62a-factory/`；命令统一装到 `/usr/local/bin/`。

## 命令约定

命令分三类：**设置 / 查看 / 验证**。针对同一测试类型（如 EEPROM 定型、LED、40-pin…）
统一命令名与调用方式，便于产线脚本对接。

**返回约定**（设置 / 验证类，供产线脚本匹配 stdout）：


| 结果  | stdout  | 退出码 |
| --- | ------- | --- |
| 成功  | `OK!`   | 0   |
| 失败  | `FAIL!` | 非 0 |


诊断/错误细节走 **stderr**，不污染 stdout 的 `OK!/FAIL!` 判定。

---

## 设备定型

**命令名称：`factory-model`**

定型参数由 MES 分配、客户端扫描板卡二维码录入，以 **JSON** 一条命令下发写入板载
EEPROM（BL24C02 @ i2c0 0x50）。JSON 键为下表字段名。设置与查看需 root。

> - `diag_version` 由产测程序在写 EEPROM 时**自动填入**，无需 MES 分配。
> - JSON 作为**单个参数**传入：MES/程序（execv）调用**无需引号**；shell 手敲无引号也可
>   （命令会重建被展开的片段），仅当值含**空格/逗号**时才需用单引号包裹整段。

### 字段清单（MES 6 参数）
（示例值为 MO-62A）

| 字段 | 说明 | 示例 |
| --- | --- | --- |
| `model` | 型号（固定值） | `Mo62A` |
| `partnumber` | 物料编号（内存） | `2G` |
| `serialnumber` | 序列号（MO62A+年+周+序号） | `MO62A2632000001` |
| `ethmac` | 以太网 MAC（MAC 池分配） | `DC:BE:04:00:00:01` |
| `oem` | 制造商（固定值） | `InHand` |
| `hwversion` | 硬件版本（随 PCB 迭代） | `V1.1` |
| `diag_version` | 诊断版本（产测自动填，非 MES） | `mo62a-diag-1.0.0` |

### 设置（下发定型参数）

全量（含全部 6 个 MES 字段 → 先擦除整片再写）：

```shell
Mo62A(factory)# factory-model {"model":"Mo62A","partnumber":"2G","serialnumber":"MO62A2632000001","ethmac":"DC:BE:04:00:00:01","oem":"InHand","hwversion":"V1.1"}
OK!
```

个别（只更新给出的字段，其余保留、不擦除）：

```shell
Mo62A(factory)# factory-model {"serialnumber":"MO62A2632000001"}
OK!
```

### 设备定型校验

读回 EEPROM 比对预期定型参数，全部匹配返回 `OK!`，否则 `FAIL!`：

```shell
Mo62A(factory)# factory-model verify {"ethmac":"DC:BE:04:00:00:01","serialnumber":"MO62A2632000001"}
OK!

Mo62A(factory)# factory-model verify {"serialnumber":"WRONG"}
FAIL!
```

### 查看

```shell
Mo62A(factory)# factory-model
TlvInfo header ver=0x01 total_len=74
CRC: stored=0xCF294793 computed=0xCF294793 OK

Type  Name                 Len  Value
--------------------------------------------------
0x21  Model                5    Mo62A
0x22  Part Number          2    2G
0x23  Serial Number        15   MO62A2632000001
0x24  Eth MAC              6    DC:BE:04:00:00:01
0x26  HW Version           4    V1.1
0x2B  OEM                  6    InHand
0x2E  Diag Version         16   mo62a-diag-1.0.0
```

（`factory-model read --json` 输出 JSON。）

### 存储与硬件

- **器件**：BL24C02 EEPROM @ i2c0 地址 `0x50` → `/sys/bus/i2c/devices/0-0050/eeprom`（256 字节）。
- **编码**：ONIE TlvInfo v1（`TlvInfo\0` 头 + version + u16 长度 + 各 TLV + `0xFE` CRC-32 尾）。
  字段→TLV type：`model`0x21 `partnumber`0x22 `serialnumber`0x23 `ethmac`0x24(MAC)
  `hwversion`0x26 `oem`0x2B `diag_version`0x2E。**`ethmac` 落在标准 Base MAC(0x24)**，
  便于 U-Boot/内核直接取用。
- **写保护**：WP 由 `R267` 默认拉高（保护），受 `GPIO1_7`（"EEP_WC"，`gpiochip2` line 7）
  控制；写入时短暂拉低放开、写完立即回置高。
- **依赖**：写入依赖 `python3-libgpiod`（已在 base 镜像中）。

---

## 整机测试单元（逐一补充接口）

下列为需做成工厂命令的整机测试单元，来源为现有 GUI 自动化测试框架
`tools/mo62a-auto-test/`（PySide6 + SSH，9 类 36 用例）。产测命令按工厂固定接口
**逐项补充**：每项确定「命令名 / 调用方式 / 逻辑与判定」后，落到 `tools/mo62a-factory/`
的 deb 里。

- **状态**：✅ 已做成工厂命令 ｜ 🟡 auto-test 已覆盖、待转工厂命令 ｜ ⬜ 尚无测试、需新写
- **类型**：设置 ｜ 查看(INFO) ｜ 验证(PASS/FAIL) ｜ 人工确认(MANUAL)

### 工厂产测菜单（权威，逐一补充）

以下 17 项为本机型工厂产测的测试单元清单，命令按工厂固定接口逐项补充。

| # | 测试单元 | 检测内容 / 判定 | 类型 | 工厂命令 | 状态 | auto-test 来源 |
|---|---|---|---|---|---|---|
| 1 | HDMI 测试 | DRM connector 状态 + 画面（人工） | 验证·人工 | 待定 | 🟡 | display/hdmi |
| 2 | Debug 调试串口测试 | ttyS2 TX↔RX 短接回环（唯一标记子串匹配） | 验证 | `com test debug` | ✅ | 新写(已完成) |
| 3 | 3.5mm 耳机音频测试 | 播放1kHz+录音 FFT SNR ≥ 10dB（需 TRRS 回环线） | 验证 | `audio test headphone` | ✅ | audio/loopback |
| 4 | MIPI CSI 摄像头测试 | 抓帧解二维码（zbar；`preview` 边显 HDMI 边解） | 验证 | `camera test csi` / `camera test csi preview` | ✅ | 新写(已完成) |
| 5 | 以太网测试 | 链路up + 千兆(1000M) + ping通 | 验证 | `net test eth [目标]` | ✅ | network/ethernet |
| 6 | FAN 风扇测试 | 停 fancontrol，0%/100% PWM 转速差 | 验证 | `fan test` | ✅ | power/fan |
| 7 | 40pin GPIO 测试 | 13 对短接双向互驱互读（补 pin23↔pin32=gpiochip2:17↔14） | 验证 | `dio detect` | ✅ | expansion/gpio_loopback |
| 8 | Wi-Fi 测试 | 扫指定 SSID 2g/5g 信号(`signal: -NN dBm`) | 验证 | `wlan 2g\|5g signal <AP>` | ✅ | network/wifi |
| 9 | 蓝牙测试 | 打印控制器 MAC（hci0 BD Address，验 RTL8821CS BT/UART-H5） | 验证 | `bt detect` | ✅ | network/bluetooth |
| 10 | SD 卡测试 | 容量≥16G + 顺序读≥20 + 写≥10 MB/s | 验证 | `storage test` | ✅ | storage/sdcard |
| 11 | RTC 测试（电池掉电保持） | `set` 写基准 → 断电≥10s → `check` 判断电期间 RTC 是否继续走 | 验证 | `rtc set` / `rtc check` | ✅ | rtc/pcf85363 |
| 12 | 内存测试 | 真实 DDR(DT `/memory`) vs 定型 partnumber / 指定容量 | 验证 | `mem detect [2G\|4G\|8G]` | ✅ | storage/ddr |
| 13 | LED 测试 | `on all red/green` 点亮单色(人工目视) + `off all` 恢复 led-status | 验证·人工 | `led on all <red\|green>` / `led off all` | ✅ | display/led |
| 14 | 按键测试 | 即时读电源键状态(EVIOCGKEY, press/release) | 验证 | `key power status <press\|release>` | ✅ | power/button |
| 15 | USB 测试 | 已拆为 ↓ Hub(16) + U盘(17) | — | 见 16/17 | ✅ | usb/hub |
| 16 | USB Hub 测试 | `lsusb` 有 0424:2514(USB2514) | 验证 | `usb hub detect` | ✅ | usb/hub |
| 17 | U盘测试(4个) | 枚举≥4 且每个裸设备只读 4MB 通过(非破坏,不挑FS) | 验证 | `usb driver detect` | ✅ | usb（storage） |
| 18 | HDMI 显示测试 | Mo62A 输出二维码 -> 采集卡 -> PC scp 回传 -> Mo62A 解码比对 | 验证 | `display hdmi detect [expected_qr]` | ✅ | 新写 |
| 19 | HDMI 音频测试 | Mo62A HDMI 播放 1kHz -> 采集卡 -> PC scp 回传 WAV -> FFT 验证 | 验证 | `audio test hdmi` | ✅ | 新写 |

> 说明：**Debug 调试串口**已实现为 `com test debug`（真机验证 OK）；其余 15 项可复用
> `tools/mo62a-auto-test/` 现有测试逻辑，转成设备端工厂命令即可。下方按 auto-test 现有实现
> 列出明细，供实现参考。EEPROM 读写已由「设备定型 `factory-model`」覆盖，不单列。
>
> **#2 Debug 串口 SOP（回环头上电时序坑）**：工厂是**先装 ttyS2 TX↔RX 短接头再上电**。
> 启动阶段设备日志经短接回灌到自身输入，会卡在两处“读串口”的关口：① U-Boot autoboot 的
> `AUTOBOOT_DELAY_STR="d"`（回灌的 `d` 打断倒计时）；② extlinux 的 `menu title` 触发交互菜单
> `Enter choice:`（回灌喂非法输入 → `not found` 死循环，永不引导）。
> **已修复**：U-Boot defconfig 置 `CONFIG_AUTOBOOT_DELAY_STR=""`（保留 Ctrl-C 调试）；
> `extlinux.conf` 去掉 `menu title`（prompt=0 → 直接引导 `default`，不读串口，overlay 改用
> `bootcfg switch` 切）。修复后带短接头冷启即可正常进系统；`com test debug` 裸开 `/dev/ttyS2`、
> 运行时自停并在 finally 自恢复 `serial-getty@ttyS2`，不受影响。
>
> **全局设计铁律**：① 只查焊接/装配（软件兼容/Bug 属研发闭环）；② 零持久化——测试改动只在
> 运行时（RAM），不写 SD：临时文件一律放 **`/dev/shm`**（Linux 恒为 tmpfs/内存，与镜像 /tmp
> 挂载方式无关），且无论成败/异常/中断都在 finally/信号处理里还原（如 `com` 恢复 getty/printk、
> `audio` 用 `alsactl` 复原 mixer）。

---

### 定型


| 单元                    | 检测内容 / 判定    | 类型       | 工厂命令            | 状态  |
| --------------------- | ------------ | -------- | --------------- | --- |
| 设备定型（EEPROM ONIE TLV） | 写入/读取/校验身份信息 | 设置·查看·验证 | `factory-model` | ✅   |


### system 系统信息


| 单元     | 检测内容 / 判定                     | 类型  | 工厂命令 | 状态  |
| ------ | ----------------------------- | --- | ---- | --- |
| 固件版本   | `mo-version`                  | 查看  | 待定   | 🟡  |
| 内核版本   | `uname -r`                    | 查看  | 待定   | 🟡  |
| CPU 核数 | `nproc`                       | 查看  | 待定   | 🟡  |
| CPU 温度 | main0_thermal / thermal_zone0 | 查看  | 待定   | 🟡  |
| 运行时长   | `uptime -p`                   | 查看  | 待定   | 🟡  |


### rtc 实时时钟（PCF85363）


| 单元     | 检测内容 / 判定               | 类型  | 工厂命令 | 状态  |
| ------ | ----------------------- | --- | ---- | --- |
| RTC 器件 | `/dev/rtc0` 名含 pcf85363 | 验证  | 待定   | 🟡  |
| RTC 读取 | `hwclock -r` 年份 ≥ 2024  | 验证  | 待定   | 🟡  |
| RTC 走时 | 1s 内 RTC 与主机误差 ≤ 2s     | 验证  | 待定   | 🟡  |
| RTC 写入 | set→读回→恢复               | 验证  | 待定   | 🟡  |


### storage 存储


| 单元         | 检测内容 / 判定             | 类型  | 工厂命令 | 状态  |
| ---------- | --------------------- | --- | ---- | --- |
| DDR 容量     | device-tree memory 求和 | 查看  | 待定   | 🟡  |
| DDR 带宽     | `mbw` MEMCPY          | 查看  | 待定   | 🟡  |
| SD/eMMC 容量 | `df -h /`             | 查看  | 待定   | 🟡  |
| SD/eMMC 读  | `dd` 顺序读 64M          | 查看  | 待定   | 🟡  |
| SD/eMMC 写  | `dd` 顺序写 16M          | 查看  | 待定   | 🟡  |


### network 网络


| 单元       | 检测内容 / 判定                | 类型  | 工厂命令 | 状态  |
| -------- | ------------------------ | --- | ---- | --- |
| 以太网速率    | `/sys/class/net/*/speed` | 查看  | 待定   | 🟡  |
| 以太网 IP   | `ip -4 addr`             | 查看  | 待定   | 🟡  |
| 以太网吞吐    | iperf3（需对端）              | 查看  | 待定   | 🟡  |
| Wi-Fi 扫描 | `iw scan` BSS 计数         | 查看  | 待定   | 🟡  |
| Wi-Fi 信号 | 最强 dBm                   | 查看  | 待定   | 🟡  |
| 蓝牙扫描     | `btmgmt find` LE 计数      | 查看  | 待定   | 🟡  |
| 蓝牙信号     | 最强 RSSI                  | 查看  | 待定   | 🟡  |


### display 显示

| 单元        | 检测内容 / 判定            | 类型   | 工厂命令 | 状态  |
| --------- | -------------------- | ---- | ---- | --- |
| HDMI 显示测试 | Mo62A 输出二维码 -> 采集卡 -> PC scp 回传 PNG -> Mo62A 解码比对 | 验证 | `display hdmi detect [expected_qr]` | ✅ | 新写 |
| HDMI 显示自测 | 不依赖采集卡，直接复制参考图完成闭环验证 | 验证 | `display hdmi detect --loopback [expected_qr]` | ✅ | 新写 |
| 红色 LED    | 点亮（人工）               | 人工确认 | `led on all red` | ✅ | display/led |
| 绿色 LED    | 点亮（人工）               | 人工确认 | `led on all green` | ✅ | display/led |
| IMX219 识别 | media-ctl 有 imx219   | 查看   | `camera`（info） | ✅ | camera |
| IMX219 预览 | 预览画面（人工）             | 人工确认 | `camera test csi preview` | ✅ | camera |

**HDMI 显示测试流程**

```shell
# 产线真实流程（需 PC 采集卡回传）
Mo62A(factory)# display hdmi detect
OK!
# PC 端：采集卡抓图后 scp 到 /tmp/mo_hdmi_cap.png

# 无采集卡时的自测验证
Mo62A(factory)# display hdmi detect --loopback TEST123
OK!
```

- 不指定 `expected_qr` 时，命令自动生成随机内容。
- 参考图自动保存到 `/tmp/mo_hdmi_test.png`；真实回传目标为 `/tmp/mo_hdmi_cap.png`。
- 命令会停 `lightdm` 独占 framebuffer，测试完自动恢复。


### power 电源


| 单元          | 检测内容 / 判定         | 类型  | 工厂命令 | 状态  |
| ----------- | ----------------- | --- | ---- | --- |
| 风扇          | 0%/100% 转速差       | 验证  | 待定   | 🟡  |
| S1 电源键      | 10s 内检测 KEY_POWER | 验证  | 待定   | 🟡  |
| 电池/RTC 掉电保持 | 断电前后 RTC 走时（人工断电） | 验证  | 待定   | 🟡  |


### usb


| 单元      | 检测内容 / 判定           | 类型  | 工厂命令 | 状态  |
| ------- | ------------------- | --- | ---- | --- |
| USB Hub | `lsusb` 有 0424:2514 | 查看  | 待定   | 🟡  |
| USB 枚举  | USB 块设备数 ≥ 期望       | 验证  | 待定   | 🟡  |
| USB 读速  | 各设备并发 `dd`          | 查看  | 待定   | 🟡  |


### audio 音频

| 单元      | 检测内容 / 判定                  | 类型  | 工厂命令 | 状态  |
| ------- | -------------------------- | --- | ---- | --- |
| HDMI 音频 | Mo62A HDMI 播放 1kHz -> 采集卡 -> PC scp 回传 WAV -> FFT 验证 | 验证 | `audio test hdmi` | ✅ | 新写 |
| HDMI 音频自测 | 不依赖采集卡，直接复制参考 WAV 完成闭环验证 | 验证 | `audio test hdmi --loopback` | ✅ | 新写 |
| 耳机回环    | 播放+录音 FFT SNR ≥ 10dB（需回环线） | 验证 | `audio test headphone` | ✅ | audio/loopback |

**HDMI 音频测试流程**

```shell
# 产线真实流程（需 PC 采集卡回传）
Mo62A(factory)# audio test hdmi
OK!
# PC 端：采集卡录音后 scp 到 /tmp/mo_hdmi_audio.wav

# 无采集卡时的自测验证
Mo62A(factory)# audio test hdmi --loopback
OK!
```

- 参考音频自动保存到 `/tmp/mo_audio_test.wav`；真实回传目标为 `/tmp/mo_hdmi_audio.wav`。
- FFT 验证 1kHz 峰值相对噪声 SNR ≥ 10dB。


### expansion 40-pin 扩展


| 单元 | 检测内容 / 判定 | 类型 | 工厂命令 | 状态 |
| --- | --- | --- | --- | --- |
| GPIO 回环 | 13 对短接双向互驱互读（**40-pin 全部功能由此一并覆盖**，UART/SPI/I2C 不单列） | 验证 | `dio detect` | ✅ |


### 其它（非物理接口）

- **HDMI 测试**：已拆分为 `display hdmi detect`（视频）和 `audio test hdmi`（音频），通过 HDMI 采集卡 + PC 回传完成闭环验证；支持 `--loopback` 自测模式。
- **EEPROM 读写**：已由「设备定型 `factory-model`」覆盖（写入即读回校验），无需单列。
- **C7x DSP**：非物理接口；如需可另加推理自检（不在物理接口范围）。
- **Bluetooth**：与 Wi-Fi 同芯片、共用一根天线，RF/天线由 Wi-Fi 测试代表；默认不单列（如需覆盖 BT 的 UART/H5 通路，可加一行式 `hciconfig hci0` 在位检查）。


