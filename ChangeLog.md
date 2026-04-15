# Changelog

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
- Disable DPMS and X11 screen blanking (`xorg.conf.d/10-no-dpms.conf` +
  `lightdm.conf`) to prevent the display from blanking and becoming
  unresponsive.
- Install `imx219-preview.sh` to `/usr/local/bin` for one-command CSI camera
  preview; the script auto-detects `/dev/videoX` and the IMX219 subdev node.

### Tooling

- `mo-62a-flash.sh`: unified flash tool supporting online SD card flash (direct
  write via `/dev/sdX`) and offline image generation for balenaEtcher.
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
