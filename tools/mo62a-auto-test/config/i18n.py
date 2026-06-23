"""双语字符串管理（中文 / English）"""

from __future__ import annotations

_lang: str = "zh"

_STRINGS: dict[str, dict[str, str]] = {
    # ── 欢迎页 ────────────────────────────────────────────────────────────────
    "welcome_title":    {"zh": "Mo 62A 自动测试系统",           "en": "Mo 62A Auto Test System"},
    "welcome_subtitle": {"zh": "硬件自动化测试套件  ·  TI AM62Ax", "en": "Automated Hardware Test Suite  ·  TI AM62Ax"},
    "welcome_continue": {"zh": "继续",                          "en": "Continue"},
    "welcome_brand":    {"zh": "Mo 62A Auto Test",              "en": "Mo 62A Auto Test"},

    # ── 连接页 ────────────────────────────────────────────────────────────────
    "conn_discovery_title": {"zh": "设备发现",        "en": "Device Discovery"},
    "conn_scan_btn":        {"zh": "扫描网络",        "en": "Scan Network"},
    "conn_scanning":        {"zh": "扫描中...",       "en": "Scanning..."},
    "conn_scan_found":      {"zh": "发现 {} 台设备",  "en": "Found {} device(s)"},
    "conn_scan_none":       {"zh": "未发现设备",      "en": "No devices found"},
    "conn_scan_error":      {"zh": "扫描失败: {}",    "en": "Scan failed: {}"},
    "conn_form_title":      {"zh": "连接配置",        "en": "Connection"},
    "conn_ip_label":        {"zh": "IP 地址",         "en": "IP Address"},
    "conn_user_label":      {"zh": "用户名",          "en": "Username"},
    "conn_pwd_label":       {"zh": "密 码",           "en": "Password"},
    "conn_connecting":      {"zh": "连接中...",       "en": "Connecting..."},
    "conn_failed":          {"zh": "连接失败: {}",    "en": "Connect failed: {}"},
    "conn_history_title":   {"zh": "历史连接",        "en": "Recent Connections"},
    "conn_history_empty":   {"zh": "暂无历史记录",    "en": "No history yet"},
    "conn_no_password":     {"zh": "(无密码)",        "en": "(no password)"},

    # ── 按钮 ──────────────────────────────────────────────────────────────────
    "btn_back":     {"zh": "返 回", "en": "Back"},
    "btn_continue": {"zh": "继 续", "en": "Continue"},

    # ── 运行页 ────────────────────────────────────────────────────────────────
    "run_running":      {"zh": "正在运行: {}",   "en": "Running: {}"},
    "run_waiting":      {"zh": "等待",           "en": "Waiting"},
    "run_col_name":     {"zh": "测试项",         "en": "Test"},
    "run_col_status":   {"zh": "状态",           "en": "Status"},
    "run_col_message":  {"zh": "结果信息",       "en": "Message"},
    "run_col_duration": {"zh": "耗时",           "en": "Duration"},
    "run_summary":      {"zh": "INFO: {}   PASS: {}   FAIL: {}   SKIP: {}",
                         "en": "INFO: {}   PASS: {}   FAIL: {}   SKIP: {}"},
    "run_btn_rerun":    {"zh": "重新运行",        "en": "Re-run"},
    "run_btn_report":   {"zh": "保存报告",        "en": "Save Report"},
    "run_leave_title":  {"zh": "测试未完成",      "en": "Tests Running"},
    "run_leave_msg":    {"zh": "测试尚未完成，确定离开吗？",
                         "en": "Tests are still running. Leave anyway?"},
    "run_report_saved": {"zh": "报告已保存",      "en": "Report saved"},
    "run_report_fail":  {"zh": "保存失败: {}",    "en": "Save failed: {}"},
    "manual_confirm_title": {"zh": "人工确认",   "en": "Manual Confirmation"},
    "manual_yes":       {"zh": "是",             "en": "Yes"},
    "manual_no":        {"zh": "否",             "en": "No"},
    "manual_hdmi_login": {"zh": "处于登录界面？",                "en": "At the login screen?"},

    # ── 报告 ───────────────────────────────────────────────────────────────────
    "report_title":         {"zh": "Mo 62A 自动测试报告",        "en": "Mo 62A Auto Test Report"},
    "report_device":        {"zh": "设备",                        "en": "Device"},
    "report_ip":            {"zh": "IP",                          "en": "IP"},
    "report_time":          {"zh": "时间",                        "en": "Time"},
    "report_total_duration":{"zh": "总耗时",                      "en": "Total Duration"},
    "report_col_test":      {"zh": "测试项",                      "en": "Test"},
    "report_col_status":    {"zh": "状态",                        "en": "Status"},
    "report_col_message":   {"zh": "结果信息",                    "en": "Message"},
    "report_col_duration":  {"zh": "耗时",                        "en": "Duration"},
    "manual_led_red":   {"zh": "红色 LED 常亮？",               "en": "Red LED steady on?"},
    "manual_led_green": {"zh": "绿色 LED 常亮？",               "en": "Green LED steady on?"},
    "manual_imx219":    {"zh": "摄像头画面？",                  "en": "Camera image?"},
    "manual_button_prompt":      {"zh": "请短按 S1 按键并松开（<2秒）", "en": "Press and release S1 button (<2s)"},
    "manual_battery_disconnect": {"zh": "请对设备断电",                  "en": "Please disconnect device power"},
    "manual_battery_reconnect":  {"zh": "请对设备上电",                  "en": "Please reconnect device power"},

    # ── 测试选择页 ────────────────────────────────────────────────────────────
    "sel_title":         {"zh": "选择测试项", "en": "Select Tests"},
    "sel_summary_title": {"zh": "已选",       "en": "Selected"},
    "sel_count_sub":     {"zh": "{} / {} 项", "en": "{} / {} items"},
    "sel_select_all":    {"zh": "全 选",      "en": "Select All"},
    "sel_deselect_all":  {"zh": "全不选",     "en": "Deselect All"},

    # ── 测试分类 ──────────────────────────────────────────────────────────────
    "cat_system":    {"zh": "系统",       "en": "System"},
    "cat_storage":   {"zh": "存储",       "en": "Storage"},
    "cat_network":   {"zh": "网络",       "en": "Network"},
    "cat_display":   {"zh": "显示",       "en": "Display"},
    "cat_audio":     {"zh": "音频",       "en": "Audio"},
    "cat_camera":    {"zh": "摄像头",     "en": "Camera"},
    "cat_expansion": {"zh": "扩展接口",   "en": "Expansion"},
    "cat_power":     {"zh": "电源",        "en": "Power"},
    "cat_rtc":       {"zh": "时钟",       "en": "Clock"},
    "cat_usb":       {"zh": "USB",        "en": "USB"},

    # ── 测试项名称 ────────────────────────────────────────────────────────────
    # 系统
    "tn_firmware_version": {"zh": "固件版本",  "en": "Firmware Version"},
    "tn_kernel_version":   {"zh": "内核版本",  "en": "Kernel Version"},
    "tn_cpu_cores":        {"zh": "CPU 核数",  "en": "CPU Cores"},
    "tn_cpu_temp":         {"zh": "CPU 温度",  "en": "CPU Temperature"},
    "tn_uptime":           {"zh": "运行时间",  "en": "Uptime"},
    # 存储
    "tn_ddr_capacity":     {"zh": "DDR 容量",  "en": "DDR Capacity"},
    "tn_ddr_bandwidth":    {"zh": "DDR 带宽",  "en": "DDR Bandwidth"},
    "tn_sd_capacity":      {"zh": "SD 卡容量", "en": "SD Capacity"},
    "tn_sd_read":          {"zh": "SD 读速度", "en": "SD Read Speed"},
    "tn_sd_write":         {"zh": "SD 写速度", "en": "SD Write Speed"},
    # 网络
    "tn_eth_speed":        {"zh": "ETH 速率",  "en": "ETH Speed"},
    "tn_eth_iperf":        {"zh": "ETH 打流",  "en": "ETH Throughput"},
    "tn_wifi_scan":        {"zh": "WLAN 扫描", "en": "WLAN Scan"},
    "tn_wifi_signal":      {"zh": "WLAN 信号", "en": "WLAN Signal"},
    "tn_bt_scan":          {"zh": "BLE 扫描",  "en": "BLE Scan"},
    "tn_bt_signal":        {"zh": "BLE 信号",  "en": "BLE Signal"},
    # 显示
    "tn_hdmi_status":      {"zh": "HDMI 状态",   "en": "HDMI Status"},
    "tn_hdmi_screen":      {"zh": "HDMI 画面",   "en": "HDMI Screen"},
    "tn_imx219_detect":    {"zh": "IMX219 检测", "en": "IMX219 Detect"},
    "tn_imx219_capture":   {"zh": "IMX219 显示", "en": "IMX219 Display"},
    # RTC
    "tn_rtc_device":       {"zh": "RTC 设备",  "en": "RTC Device"},
    "tn_rtc_read":         {"zh": "当前时间",  "en": "Current Time"},
    "tn_rtc_tick":         {"zh": "时钟走动",  "en": "Clock Tick"},
    "tn_rtc_write":        {"zh": "写入验证",  "en": "Write Verify"},
    "tn_rtc_battery":      {"zh": "电池保持",  "en": "Battery Hold"},
    # 音频
    "tn_hdmi_audio":         {"zh": "HDMI 音频",    "en": "HDMI Audio"},
    "tn_headphone_loopback": {"zh": "3.5mm 环回",   "en": "3.5mm Loopback"},
    # 摄像头（待实现）
    "tn_cam_pipeline":     {"zh": "摄像头管道", "en": "Camera Pipeline"},
    "tn_cam_capture":      {"zh": "帧捕获",    "en": "Frame Capture"},
    # 扩展接口
    "tn_gpio_loopback":    {"zh": "电平翻转",     "en": "GPIO Loopback"},
    "gpio_pass_summary":   {"zh": "{}对通过",    "en": "{} pairs passed"},
    "gpio_fail_summary":   {"zh": "失败：{}",     "en": "Failed: {}"},
    "gpio_unknown_output": {"zh": "未知输出：{}", "en": "Unknown output: {}"},
    "gpio_write_script_fail": {"zh": "写入 GPIO 测试脚本失败：{}", "en": "Failed to write GPIO test script: {}"},
    "gpio_run_fail":       {"zh": "GPIO 测试执行失败：{}", "en": "GPIO test execution failed: {}"},
    # 电源/LED
    "tn_fan_control":      {"zh": "风扇控制",  "en": "Fan Control"},
    "tn_led_red":          {"zh": "红色 LED",  "en": "Red LED"},
    "tn_led_green":        {"zh": "绿色 LED",  "en": "Green LED"},
    "tn_button":           {"zh": "按键测试",  "en": "Button Test"},
    "tn_rtc_battery":      {"zh": "电池保持",  "en": "Battery Hold"},
    # USB
    "tn_usb_hub":          {"zh": "USB Hub",   "en": "USB Hub"},
    "tn_usb_enum":         {"zh": "USB 枚举",  "en": "USB Enumeration"},
    "tn_usb_read":         {"zh": "USB 速率",  "en": "USB Read Speed"},

    # ── 测试结果消息（中英文动态切换）────────────────────────────────────────
    # 系统
    "msg_mo_version_unavailable": {"zh": "mo-version 命令不可用", "en": "mo-version command unavailable"},
    "msg_kernel_read_fail":       {"zh": "无法读取内核版本", "en": "Cannot read kernel version"},
    "msg_cpu_cores_fail":         {"zh": "无法读取 CPU 核数", "en": "Cannot read CPU core count"},
    "msg_cpu_cores":              {"zh": "{} 核", "en": "{} cores"},
    "msg_cpu_temp_fail":          {"zh": "无法读取 CPU 温度", "en": "Cannot read CPU temperature"},
    "msg_uptime_fail":            {"zh": "无法读取运行时间", "en": "Cannot read uptime"},

    # 存储
    "msg_sd_capacity_fail":       {"zh": "无法读取 SD 卡容量信息", "en": "Cannot read SD card capacity"},
    "msg_sd_df_format":           {"zh": "df 输出格式异常：{}", "en": "df output format abnormal: {}"},
    "msg_sd_read_fail":           {"zh": "dd 读取失败：{}", "en": "dd read failed: {}"},
    "msg_sd_parse_fail":          {"zh": "无法解析 dd 输出：{}", "en": "Cannot parse dd output: {}"},
    "msg_sd_write_fail":          {"zh": "dd 写入失败：{}", "en": "dd write failed: {}"},
    "msg_ddr_capacity_fail":      {"zh": "无法读取 DDR 容量", "en": "Cannot read DDR capacity"},
    "msg_ddr_capacity":           {"zh": "{} GB", "en": "{} GB"},
    "msg_ddr_mbw_missing":        {"zh": "mbw 未安装，请执行：sudo apt-get install -y mbw", "en": "mbw not installed, run: sudo apt-get install -y mbw"},
    "msg_ddr_mbw_fail":           {"zh": "mbw 执行失败", "en": "mbw execution failed"},
    "msg_ddr_mbw_parse_fail":     {"zh": "无法解析 mbw 输出", "en": "Cannot parse mbw output"},
    "msg_ddr_speed":              {"zh": "{} MB/s", "en": "{} MB/s"},

    # RTC
    "msg_rtc_device_missing":     {"zh": "/dev/rtc0 不存在", "en": "/dev/rtc0 does not exist"},
    "msg_rtc_driver_mismatch":    {"zh": "驱动名称异常：{}（期望 pcf85363）", "en": "Driver name abnormal: {} (expected pcf85363)"},
    "msg_rtc_read_fail":          {"zh": "hwclock -r 失败：{}", "en": "hwclock -r failed: {}"},
    "msg_rtc_time_abnormal":      {"zh": "时间异常（可能掉电）：{}", "en": "Time abnormal (possible power loss): {}"},
    "msg_rtc_sysfs_fail":         {"zh": "无法读取 RTC sysfs 时间", "en": "Cannot read RTC sysfs time"},
    "msg_rtc_parse_fail":         {"zh": "时间格式解析失败：{} / {}", "en": "Time format parse failed: {} / {}"},
    "msg_rtc_tick_pass":          {"zh": "成功（误差 {}ms）", "en": "Success (error {}ms)"},
    "msg_rtc_tick_fail":          {"zh": "失败（误差 {}ms）", "en": "Failed (error {}ms)"},
    "msg_rtc_system_time_fail":   {"zh": "无法读取系统时间", "en": "Cannot read system time"},
    "msg_rtc_time_parse_fail":    {"zh": "解析系统时间失败：{}", "en": "Parse system time failed: {}"},
    "msg_rtc_set_fail":           {"zh": "hwclock --set 失败：{}", "en": "hwclock --set failed: {}"},
    "msg_rtc_readback_fail":      {"zh": "写入后读取失败", "en": "Readback after write failed"},
    "msg_rtc_mismatch":           {"zh": "读回时间不符：{}", "en": "Readback time mismatch: {}"},
    "msg_rtc_write_pass":         {"zh": "写入 {}，读回正确，已从系统时钟恢复", "en": "Wrote {}, readback correct, restored from system clock"},

    # 网络
    "msg_eth_iface_missing":      {"zh": "未找到以太网接口", "en": "Ethernet interface not found"},
    "msg_eth_speed_fail":         {"zh": "无法读取 {} 速率", "en": "Cannot read {} speed"},
    "msg_eth_link_down":          {"zh": "{} 链路断开", "en": "{} link down"},
    "msg_eth_ip_missing":         {"zh": "{} 未分配 IP 地址", "en": "{} has no IP address"},
    "msg_iperf3_device_missing":  {"zh": "设备上未安装 iperf3，请执行：sudo apt-get install -y iperf3", "en": "iperf3 not installed on device, run: sudo apt-get install -y iperf3"},
    "msg_iperf3_host_missing":    {"zh": "测试主机未安装 iperf3，请执行：sudo apt-get install -y iperf3", "en": "iperf3 not installed on host, run: sudo apt-get install -y iperf3"},
    "msg_iperf3_connect_fail":    {"zh": "iperf3 连接失败：{}", "en": "iperf3 connection failed: {}"},
    "msg_bt_ctrl_missing":        {"zh": "未找到蓝牙控制器 hci0", "en": "Bluetooth controller hci0 not found"},
    "msg_bt_count":               {"zh": "发现 {} 个设备", "en": "Found {} devices"},
    "msg_bt_no_rssi":             {"zh": "未获取到 RSSI 数据", "en": "No RSSI data obtained"},
    "msg_bt_strongest":           {"zh": "最强 {} dBm", "en": "Strongest {} dBm"},
    "msg_wifi_iface_missing":     {"zh": "未找到 Wi-Fi 接口", "en": "Wi-Fi interface not found"},
    "msg_wifi_count":             {"zh": "发现 {} 个热点", "en": "Found {} hotspots"},
    "msg_wifi_no_signal":         {"zh": "未解析到信号强度", "en": "No signal strength parsed"},
    "msg_wifi_strongest":         {"zh": "最强 {} dBm", "en": "Strongest {} dBm"},

    # USB
    "msg_usb_lsusb_missing":      {"zh": "lsusb 命令不可用", "en": "lsusb command unavailable"},
    "msg_usb_hub_not_found":      {"zh": "未找到 USB2514 Hub（{}）", "en": "USB2514 Hub not found ({})"},
    "msg_usb_count":              {"zh": "{} 个设备", "en": "{} devices"},
    "msg_usb_count_expected":     {"zh": "{} 个设备（期望 {}）", "en": "{} devices (expected {})"},
    "msg_usb_none":               {"zh": "未检测到任何 USB 外设", "en": "No USB peripherals detected"},
    "msg_usb_block_missing":      {"zh": "未找到 USB 块设备", "en": "No USB block device found"},
    "msg_usb_dev_fail":           {"zh": "{}:失败", "en": "{}:failed"},
    "msg_usb_total":              {"zh": "总带宽: {} MB/s", "en": "Total bandwidth: {} MB/s"},
    "msg_usb_fail_short":         {"zh": "失败", "en": "failed"},

    # 音频
    "msg_hdmi_not_connected":     {"zh": "HDMI 未连接，跳过音频测试", "en": "HDMI not connected, skip audio test"},
    "msg_hdmi_card_missing":      {"zh": "Card 1（HDMI）未找到", "en": "Card 1 (HDMI) not found"},
    "msg_hdmi_audio_play":        {"zh": "{}Hz {}% 音量 {}s", "en": "{}Hz {}% volume {}s"},
    "msg_gst_play_fail":          {"zh": "{}", "en": "{}"},
    "msg_gst_play_generic_fail":  {"zh": "GStreamer 播放失败", "en": "GStreamer playback failed"},
    "msg_headphone_card_missing": {"zh": "Card 0（AM62Ax-SKEVM）未找到", "en": "Card 0 (AM62Ax-SKEVM) not found"},
    "msg_headphone_record_fail":  {"zh": "GStreamer 录音命令失败", "en": "GStreamer record command failed"},
    "msg_headphone_download_fail":{"zh": "下载录音文件失败：{}", "en": "Download recording failed: {}"},
    "msg_headphone_data_short":   {"zh": "录音数据过短：{} 字节", "en": "Recording data too short: {} bytes"},
    "msg_headphone_numpy_missing":{"zh": "需要 numpy（pip install numpy）", "en": "numpy required (pip install numpy)"},
    "msg_headphone_pass":         {"zh": "1kHz SNR {}  录音→{}", "en": "1kHz SNR {}  Recording→{}"},
    "msg_headphone_fail":         {"zh": "1kHz SNR {} < {} dB（请检查测试环回线是否正确插入）  录音→{}", "en": "1kHz SNR {} < {} dB (check loopback cable)  Recording→{}"},

    # 显示
    "msg_imx219_media_missing":   {"zh": "IMX219 未出现在 media 拓扑中", "en": "IMX219 not in media topology"},
    "msg_imx219_overlay_missing": {"zh": "未找到含 {} 的 extlinux label，请手动选择摄像头 overlay 重启", "en": "No extlinux label containing {}, please select camera overlay and reboot"},
    "msg_imx219_preview_fail":    {"zh": "启动摄像头预览失败：{}", "en": "Camera preview start failed: {}"},
    "msg_imx219_user_yes":        {"zh": "用户确认看到摄像头画面", "en": "User confirmed camera image"},
    "msg_imx219_user_no":         {"zh": "用户确认未看到摄像头画面", "en": "User did not see camera image"},
    "msg_led_red_yes":            {"zh": "用户确认红灯亮", "en": "User confirmed red LED lit"},
    "msg_led_red_no":             {"zh": "用户确认红灯未亮", "en": "User confirmed red LED not lit"},
    "msg_led_green_yes":          {"zh": "用户确认绿灯亮", "en": "User confirmed green LED lit"},
    "msg_led_green_no":           {"zh": "用户确认绿灯未亮", "en": "User confirmed green LED not lit"},
    "msg_hdmi_drm_missing":       {"zh": "未找到 HDMI DRM 节点", "en": "HDMI DRM node not found"},
    "msg_hdmi_user_yes":          {"zh": "用户确认显示登录界面", "en": "User confirmed login screen displayed"},
    "msg_hdmi_user_no":           {"zh": "用户确认未显示登录界面", "en": "User confirmed login screen not displayed"},

    # 电源
    "msg_fan_hwmon_missing":      {"zh": "未找到 pwmfan hwmon 设备", "en": "pwmfan hwmon device not found"},
    "msg_fan_rpm_unreadable":     {"zh": "无法读取 RPM：{}", "en": "Cannot read RPM: {}"},
    "msg_fan_rpm_low":            {"zh": "{}（100% RPM 低于 {}）", "en": "{} (100% RPM below {})"},
    "msg_fan_rpm_not_higher":     {"zh": "{}（100% 转速未高于 0%）", "en": "{} (100% RPM not higher than 0%)"},
    "msg_button_evdev_missing":   {"zh": "未找到 tps6594-pwrbutton 输入设备", "en": "tps6594-pwrbutton input device not found"},
    "msg_button_write_fail":      {"zh": "写入 watcher 脚本失败：{}", "en": "Failed to write watcher script: {}"},
    "msg_button_start_fail":      {"zh": "启动 watcher 失败：{}", "en": "Failed to start watcher: {}"},
    "msg_button_detected":        {"zh": "检测到 KEY_POWER 事件（{}s）", "en": "KEY_POWER event detected ({}s)"},
    "msg_button_timeout":         {"zh": "{}s 内未检测到按键事件", "en": "No button event detected within {}s"},
    "msg_battery_rtc_read_fail":  {"zh": "无法读取初始 RTC 时间", "en": "Cannot read initial RTC time"},
    "msg_battery_no_poweroff":    {"zh": "未检测到设备断电，设备仍然可 ping 通", "en": "Device power-off not detected, still pingable"},
    "msg_battery_reconnect_fail": {"zh": "上电后 60s 内未 ping 通设备，连接失败", "en": "Device not pingable within 60s after power-on"},
    "msg_battery_rtc_readback_fail":{"zh": "上电后无法读取 RTC 时间", "en": "Cannot read RTC time after power-on"},
    "msg_battery_pass":           {"zh": "成功（断电 {}s，RTC 走时 {}s，误差 {}s）", "en": "Success (power-off {}s, RTC elapsed {}s, error {}s)"},
    "msg_battery_fail":           {"zh": "失败（断电 {}s，RTC 走时 {}s，误差 {}s — 电池可能失效）", "en": "Failed (power-off {}s, RTC elapsed {}s, error {}s — battery may be dead)"},
}


def set_lang(lang: str) -> None:
    global _lang
    assert lang in ("zh", "en"), f"Unsupported language: {lang}"
    _lang = lang


def get_lang() -> str:
    return _lang


def t(key: str, *args) -> str:
    """获取当前语言的字符串，支持 format 占位符。"""
    entry = _STRINGS.get(key)
    if entry is None:
        return key
    text = entry.get(_lang, entry.get("en", key))
    return text.format(*args) if args else text
