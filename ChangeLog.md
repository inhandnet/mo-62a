# Changelog

## v1.0.2 — 2026-04-17

### Kernel & Device Tree

#### HDMI DPMS Wake Fix (SiI9022A)
- Fix `sii902x_bridge_atomic_enable()`: move the 20 ms TMDS PLL stabilisation
  delay (`msleep(20)`) unconditionally before `PWR_DWN` is cleared. Previously
  it was gated on `mode.clock`, which is 0 after a module hot-reload (DRM does
  not re-issue `mode_set` when only `active_changed`), causing the PLL to time
  out and HDMI output to stay dark after DPMS wake.
- Add CRTC-state fallback in `atomic_enable`: if `mode.clock` is still 0 after
  the delay (e.g. module hot-reload without reboot), read the adjusted mode from
  `bridge->encoder->crtc->state` so TPI video registers can still be programmed.
- Refactor TPI video register programming into a new `sii902x_apply_mode()` helper;
  cache the adjusted display mode in `struct sii902x` so it survives power-cycle
  and DPMS cycles without requiring `mode_set` to be re-issued.
- Add `reset-gpios = <&main_gpio1 3 GPIO_ACTIVE_LOW>` to the `sii9022` DTS node
  and a dedicated `sii9022_reset_pins` pinmux group (`RGMII2_RD0 / GPIO1_3` as
  `PIN_OUTPUT`), enabling the driver to assert HDMI_RSTn at probe/remove.

#### EEPROM BL24C02F Driver Support
- Add `eeprom@50` I2C device node to `&main_i2c1` in `k3-am62a7-mo-62a.dts`:
  `compatible = "atmel,24c02"`, address 0x50, page size 16 bytes,
  `wp-gpios = <&main_gpio1 7 GPIO_ACTIVE_HIGH>`. The `at24` driver
  (`CONFIG_EEPROM_AT24=m`) is loaded automatically at boot via udev and exposes
  the EEPROM as `/sys/bus/i2c/devices/1-0050/eeprom` (256 bytes, root read/write).
- Change pad 0x0194 (`MCASP0_AXR3`, ball C19) in `gpio1_pins_default` from
  `PIN_INPUT` to `PIN_OUTPUT`. The EEP_WC (write-control) signal is pulled to
  VCC_3V3_SYS via R267 (10 kΩ). With the pad as input the WP pin floated high,
  write-protecting the EEPROM. Reconfiguring it as output allows the `at24`
  driver to assert the pad low (write enabled) via `wp-gpios`.

#### TIDSS DPMS Wake Black Screen Fix (tidss_plane.c)
- Fix `tidss_plane_atomic_update()` in `drivers/gpu/drm/tidss/tidss_plane.c`: add
  `dispc_plane_enable(true)` for visible planes in addition to `dispc_plane_setup()`.
  **Root cause**: After DPMS Off, `tidss_runtime_put()` drops the PM refcount to zero.
  After the autosuspend delay (1 s), `dispc_runtime_suspend()` disables the functional
  clock, power-cycling the DSS hardware. On DPMS On, `dispc_runtime_resume()` calls
  `dispc_initial_config()` → `dispc_k3_plane_init()`, which resets
  `DISPC_VID_ATTRIBUTES` bit 0 (VID pipeline enable) to 0. The DRM commit then calls
  `tidss_plane_atomic_update()` (which writes DMA shadow registers) but skips
  `tidss_plane_atomic_enable()` because `drm_atomic_plane_enabling()` returns false —
  the DRM state records the plane as already bound to the CRTC, so the framework does
  not recognise the need to re-enable it. With the VID pipeline disabled the overlay
  layer receives no pixel data and the display outputs black, even though SiI9022A
  delivers valid HDMI sync (monitor LED shows white/signal-present). The fix mirrors
  the first-activation path: unconditionally re-enable the pipeline inside
  `atomic_update()` whenever the plane is visible, making the call idempotent for
  normal page-flips and correct for the DPMS-wake power-cycle case.

### Rootfs

#### DPMS Configuration and Wake Daemon
- Add `/etc/xdg/autostart/enable-dpms.desktop` to the rootfs overlay: runs at every
  XFCE session start with `xset +dpms; xset dpms 0 0 600; xset s off; xset s noblank;
  dpms-wakeup &`. Enables DPMS with a 10-minute Off timeout (Standby/Suspend
  disabled), suppresses the X screensaver, and launches the `dpms-wakeup` daemon.
- Add `dpms-wakeup` Python daemon (`/usr/local/bin/dpms-wakeup`): monitors all
  `/dev/input/event*` nodes with `select()` and calls `xset dpms force on` on any
  keyboard or mouse activity while the display is in DPMS Off state. Includes a 2-second
  cooldown and an event drain loop to suppress keyboard auto-repeat storms. Addresses
  the Xorg limitation where the DPMS idle timer does not automatically wake the display
  on physical input while already in the Off state.

#### DPMS Wake Reliability — xfce4-power-manager Race Fix
- Add `/etc/xdg/autostart/xfce4-power-manager.desktop` (Hidden=true) to the
  rootfs overlay. This suppresses xfce4-power-manager autostart for all XFCE
  sessions on the board.
  **Root cause**: xfce4-power-manager 4.20.0 polls the XScreenSaver idle counter
  on a fixed interval. When the display wakes from DPMS blank (keyboard press or
  `xset dpms force on`), there is a race between the XSS idle-counter reset and
  xfce4-power-manager's next poll. If the poll fires before the reset is
  processed, xfce4-power-manager sees the accumulated idle time (> `dpms-on-ac-sleep`
  threshold of 4 minutes) and immediately calls `DPMSForceLevel(Off)` again,
  causing the display to flash briefly and go dark within ~1 second of waking.
  The `presentation-mode=true` setting in its configuration does not prevent this
  behaviour in this version. With xfce4-power-manager suppressed, DPMS is owned
  entirely by the X server.

#### USB Input Device Seat Assignment
- Add `/etc/udev/rules.d/72-seat-input.rules` to the rootfs overlay.
  On AM62A with LightDM, udev does not automatically tag USB keyboard/mouse/
  touchscreen/joystick devices as `ID_SEAT=seat0` because the USB hub sits on a
  separate parent from the DSS/DRM subsystem — logind therefore omits them from
  seat0's device list, and libinput never delivers physical key/mouse events to
  Xorg. Without physical input events, the display cannot be woken from DPMS
  blank by keyboard or mouse. The new rules explicitly tag all recognised input
  device types (`ID_INPUT_KEYBOARD`, `ID_INPUT_MOUSE`, `ID_INPUT_TOUCHSCREEN`,
  `ID_INPUT_JOYSTICK`) with `ID_SEAT=seat0` and `TAG+="seat"`.

---

## v1.0.1 — 2026-04-15

### Kernel & Device Tree

#### Dual-Colour LED
- Remove non-existent blue LED node (`MCU_GPIO0_2`) — schematic carries only
  red (`MCU_GPIO0_16` / PWR_LED) and green (`MCU_GPIO0_15` / ACT_LED).
- Fix LED pinmux: change `PIN_INPUT` → `PIN_OUTPUT` for both LED pads.
- Fix LED polarity: change `GPIO_ACTIVE_HIGH` → `GPIO_ACTIVE_LOW` to match the
  transistor-driven active-low circuit (GPIO LOW = LED ON).
- Set `default-state = "on"` for the red LED so it lights immediately from
  kernel gpio-leds initialisation, giving a clear boot-in-progress indication.

### Rootfs

#### Dual-Colour LED Status Controller
- Add `led-status` Python service (`/usr/local/bin/led-status` +
  `led-status.service`): holds the red LED on until `multi-user.target` is
  reached, then turns red off and starts the green LED in breathing mode.
  Breathing period is inversely proportional to the 4-core average CPU
  utilisation — 0 % → 2 000 ms half-cycle (very slow), 100 % → 100 ms
  half-cycle (fast).

#### nginx — Missing Log Directory Fix
- Add `usr/lib/tmpfiles.d/nginx.conf` to rootfs overlay: instructs
  `systemd-tmpfiles` to create `/var/log/nginx/` (owner `www-data:adm`, mode
  0755) at boot, fixing the nginx startup failure caused by the missing
  directory in the base rootfs image.

#### fancontrol — hwmon Index Drift Fix
- Add `fancontrol-update-config` script (`/usr/local/bin/`): scans
  `/sys/class/hwmon/hwmon*/name` at each service start, locates the current
  hwmon indices for `pwmfan` and `main0_thermal`, and rewrites
  `/etc/fancontrol` accordingly — eliminating the index-drift failures that
  occurred after reboots.
- Add `fancontrol.service.d/override.conf` drop-in: runs
  `fancontrol-update-config` before the upstream `fancontrol --check` step,
  and adds `ReadWritePaths=/etc/fancontrol` to permit writing under
  `ProtectSystem=strict`.

---

## v1.0.0 — 2026-04-15

First public release of the MO-62A board support package.

### Kernel & Device Tree

#### HDMI Audio (McASP0 → SiI9022)
- Enable McASP0 as I2S transmitter: AXR2 pin routed to SiI9022 SD0, 24.576 MHz
  audio clock, stereo I2S format.
- Enable `sound-hdmi` simple-audio-card linking McASP0 (CPU DAI) and SiI9022
  (codec DAI); CPU side is bitclock/frame master.
- Add `playback-only` to the sound card node so PipeWire does not probe the
  non-existent capture direction on boot.
- Fix `sii902x_bridge_edid_read()` in the SiI9022 DRM bridge driver: read the
  EDID and set `sink_is_hdmi` via `drm_detect_hdmi_monitor()`. In bridge-chain
  mode the previous code path that set this flag was never reached, leaving the
  chip in DVI mode and suppressing audio output.
- Fix `graph_util_parse_link_direction()` in `simple-card-utils.c`: change from
  assign to OR semantics so that `playback-only` set on the sound card root node
  is not silently overwritten by subsequent cpu/codec sub-node checks.

#### RTC (PCF85263A)
- Correct the DTS `compatible` string from `nxp,pcf8563` to `nxp,pcf85263` so
  the kernel loads the `rtc-pcf85363` driver. The wrong driver misinterpreted
  all register offsets, causing spurious "low voltage detected" warnings despite
  a healthy backup battery.

#### PWM Fan Control
- Enable `main_timer7` as PWM output on pin TIMER_IO7 (J6 connector, D18).
- Add `dmtimer-pwm` and `pwm-fan` device tree nodes: 25 kHz PWM, four cooling
  states mapped to the 40–75 °C CPU temperature range.
- Expose AM62A thermal zones to hwmon sysfs (`k3_j72xx_bandgap`) so the
  `fancontrol` daemon can read CPU temperature directly.

#### IMX219 CSI Camera
- Add IMX219 camera node on I2C2: 25 MHz XCLK from AM62A CLKOUT0, GPIO0_87
  as XSHUTDOWN, VANA/VDIG/VDDL via a shared `vcc_cam` regulator.
- Recalculate IMX219 PLL registers for 25 MHz input (PREPLLCK=5,
  PLL_OP_MPY=182 → 455 MHz link, PLL_VT_MPY=91 → 182 Mpix/s).
- Add `csi0_mclk_pins` pinmux group for CLKOUT0.

#### 40-Pin Expansion Header
- Set all 40-pin header pads to GPIO mode (mux=7) by default.
- Disable `main_uart5`, `main_spi0`, `epwm1`, and `wkup_i2c0` in DTS to
  release the corresponding pads to the GPIO controller.
- Correct GPIO/EHRPWM pin assignments after schematic and hardware verification:
  EHRPWM0 A/B on pins 32/33, EHRPWM1_A on pin 36; fix GPIO1 and GPIO0 line
  numbers throughout.

### Rootfs

- Install and pre-configure `fancontrol` and `lm-sensors`; enable the
  `fancontrol` service at boot.
- Install `imx219-preview.sh` to `/usr/local/bin` for one-command CSI camera
  preview; the script auto-detects `/dev/videoX` and the IMX219 subdev node.

### Tooling

- `mo-62a-flash.sh`: unified flash tool supporting online SD card flash (direct
  write via `/dev/sdX`) and offline image generation for balenaEtcher.
  Offline images follow the naming convention
  `mo-62a-<os><ver>-<desktop>-<version>-<date>.img.zip`,
  e.g. `mo-62a-debian13.3-xfce-v1.0.0-2026-04-15.img.zip`.
  Only version and date are prompted interactively; the rest is fixed.
- `setup.sh`: streamlined host setup (OS check, dialout group, package install,
  `TI_SDK_PATH` in `~/.bashrc`, `/opt` toolchain symlink).
- WirePlumber naming rules: rename the two PipeWire audio sinks from the
  generic "Built-in Audio Stereo" to "Headphone Jack (3.5mm)" and
  "HDMI Audio Output" for clarity in volume control applications.

### Documentation

- `README.md` / `README_ZH.md`: complete board bring-up guide covering clone,
  toolchain setup, U-Boot build, kernel/DTB build, flashing, partition layout,
  40-pin GPIO usage, CSI camera, PWM fan, and HDMI audio.
- `doc/QuickStart/`: English and Chinese quick-start guides with balenaEtcher
  screenshots, verified 40-pin GPIO table, and `gpiod` v2.x usage examples.
- `doc/Schematic/`: MO-62A hardware schematic PDF.
- `doc/Chips/PCF85263A.pdf`: PCF85263A RTC datasheet.
