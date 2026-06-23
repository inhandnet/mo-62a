# iperf3 二进制说明

本目录用于放置 Windows 上打流测试所需的 `iperf3.exe`。

## 使用方法

1. 从 iperf3 官方获取 Windows 可执行文件：
   - 官方下载：https://iperf.fr/iperf-download.php
   - 选择 Windows 64-bit 版本，解压得到 `iperf3.exe`。

2. 将 `iperf3.exe` 复制到本目录：
   ```
   tools/mo62a-auto-test/bin/iperf3.exe
   ```

3. 运行测试框架时，`ethernet.py` 会自动优先使用本目录下的 `iperf3.exe`。

## 注意

- 不要提交 `iperf3.exe` 到 git。本目录下的 `.gitignore` 已排除 `*.exe`。
- Linux/macOS 用户不需要放置此文件，系统 PATH 中的 `iperf3` 会被自动使用。
- 如果 Windows 上没有在本目录放置 `iperf3.exe`，也可以将 `iperf3.exe` 所在目录添加到系统 PATH，测试框架会尝试查找。
