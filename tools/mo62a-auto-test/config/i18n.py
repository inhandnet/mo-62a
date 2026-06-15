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
    "tn_sd_read":          {"zh": "SD 卡读速", "en": "SD Read Speed"},
    "tn_sd_write":         {"zh": "SD 卡写速", "en": "SD Write Speed"},
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
    "tn_imx219_capture":   {"zh": "IMX219 抓帧", "en": "IMX219 Capture"},
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
    # 扩展接口（待实现）
    "tn_gpio_output":      {"zh": "GPIO 输出", "en": "GPIO Output"},
    "tn_uart_loopback":    {"zh": "UART 回环", "en": "UART Loopback"},
    "tn_spi_loopback":     {"zh": "SPI 回环",  "en": "SPI Loopback"},
    "tn_pwm_output":       {"zh": "PWM 输出",  "en": "PWM Output"},
    # 电源/LED
    "tn_fan_control":      {"zh": "风扇控制",  "en": "Fan Control"},
    "tn_led_red":          {"zh": "红色 LED",  "en": "Red LED"},
    "tn_led_green":        {"zh": "绿色 LED",  "en": "Green LED"},
    "tn_button":           {"zh": "按键测试",  "en": "Button Test"},
    # USB（待实现）
    "tn_usb_hub":          {"zh": "USB Hub",   "en": "USB Hub"},
    "tn_usb_enum":         {"zh": "USB 枚举",  "en": "USB Enumeration"},
    "tn_usb_read":         {"zh": "USB 读速",  "en": "USB Read Speed"},
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
