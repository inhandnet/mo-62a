# Changelog

## v1.0.7 — 2026-06-24

### Kernel & Device Tree

#### Docker runtime support

- Enable BPF, cgroup controllers and seccomp in the RT kernel fragment so Docker
  can run with its default security profile:
  - `CONFIG_BPF_SYSCALL=y`, `CONFIG_BPF_JIT=y`, `CONFIG_CGROUP_BPF=y`
  - `CONFIG_MEMCG`, `CONFIG_BLK_CGROUP`, `CONFIG_CGROUP_PIDS`,
    `CONFIG_CGROUP_FREEZER`, `CONFIG_CPUSETS`, `CONFIG_CGROUP_CPUACCT`
  - `CONFIG_SECCOMP=y`, `CONFIG_SECCOMP_FILTER=y`
- Update `am62ax_mo_62a_defconfig`:
  - Build `CONFIG_OVERLAY_FS=y` for Docker `overlay2`.
  - Enable nftables / masquerade support required by Docker networking.
  - Enable `CONFIG_CRYPTO_USER_API_SKCIPHER` and `CONFIG_CRYPTO_USER_API_AEAD`
    to expose kernel crypto via AF_ALG.

#### Wi-Fi monitor mode

- Enable `CONFIG_WIFI_MONITOR=y` in the Realtek `rtl8821cs` SDIO driver so the
  `monitor` interface type is registered with cfg80211.
- Silence two over-strict `rtw_warn_on(1)` assertions triggered during 5 GHz
  association / interface type changes; the warnings were non-fatal but tainted
  the kernel.

### External Drivers

#### cryptodev Linux 6.12 compatibility

- Replace the `register_sysctl()` based verbosity control in
  `cryptodev-module-1.14` with `proc_create()`, eliminating the
  `sysctl table check failed` warning on Linux 6.12.

### Flashing & Build

#### Automatic external-driver build

- `bin/mo-62a-flash.sh` now iterates over `board-support/extra-drivers/*/`
  during rootfs creation, cross-compiles each driver against the selected
  kernel source tree and installs the modules into the target rootfs.
- This makes the cryptodev module part of the flashed image automatically.

#### First-boot `.deb` install hook

- Extend the existing first-boot service to install any `.deb` packages placed
  in `/usr/local/share/mo-62a/prebuilt-deb/` before the root partition is
  resized. The directory is shipped empty; customers can drop their own debs
  there if needed.

### Root Filesystem

#### Docker enabled by default

- Add a systemd enable symlink for `docker.service` in `rootfs-overlay` so the
  Docker daemon starts on first boot.

#### tcpdump preinstalled

- Preinstall `tcpdump` (and `libpcap`) in the base Debian rootfs so 802.11
  monitor-mode packet capture works out of the box without requiring network
  access on the target.

### Factory Test Tool

#### Windows host support

- Port `tools/mo62a-auto-test/` to run on Windows PCs (the core framework uses
  paramiko and works cross-platform).
- Disable the ping-based network test when running with the command backend on
  Windows.
- Remove the obsolete `tools/mo62a-tester/` prototype.

## v1.0.6 — 2026-06-18

### Edge AI — On-Device C/C++ SDK

#### C/C++ Edge AI development SDK (compile inference programs on the board)

- Ship a complete on-device C/C++ Edge AI SDK so customers can build and debug
  their own TIDL / C7x inference programs directly on the board — no
  cross-compilation environment required:
  - Headers under `/usr/include/edgeai/` (edgeai-dl-inferer API, TFLite / ONNX
    Runtime, and the TI app-utils headers).
  - Static libraries under `/usr/lib/edgeai/` (`edgeai_dl_inferer` / `pre` /
    `post` plus the prebuilt TFLite stack).
  - A CMake package at `/usr/lib/cmake/EdgeAI/EdgeAIConfig.cmake` exposing a
    single `EdgeAI::edgeai` target. A consumer project links the whole stack
    (TIDL ONNX Runtime, tivision\_apps, OpenCV, GStreamer, …) with one
    `find_package(EdgeAI)` + `target_link_libraries(... EdgeAI::edgeai)` — no
    manual include/library paths.
- Add example projects under `/usr/share/edgeai-cpp-examples/`:
  - `hello_inference/` — minimal load-model + single-inference sample (headless).
  - `app_edgeai/` — full camera → inference → HDMI pipeline source.
  - `configs/` (CSI + USB) and `DEV_GUIDE.md`.
- Verified on hardware: `hello_inference` compiles on the board via
  `find_package(EdgeAI)` and runs inference with all model nodes offloaded to the
  C7x DSP.

#### Unified `edgeai-demo` launcher (Python + C/C++, CSI + USB)

- Rework `edgeai-demo` into a single entry point that drives both the **Python**
  and **C/C++** backends with either a **CSI** or **USB** camera from the same
  YAML config, in interactive and command-line modes:
  - `edgeai-demo run <model> --backend python|cpp --camera csi|usb`
  - Interactive mode prompts for camera → model → backend.
- USB camera support: auto-select the real capture node (skip UVC metadata-only
  nodes) and pick a supported pixel format (MJPG → jpeg, otherwise YUYV), fixing a
  GStreamer pipeline failure on USB input.

#### C/C++ `app_edgeai` — display title

- Make the `tiperfoverlay` `main-title` configurable from YAML and default it
  off, removing the hard-coded "Texas Instruments Edge AI" banner so the C/C++
  demo matches the Python demo's clean overlay.

### Root Filesystem

#### C/C++ development packages preinstalled

- Preinstall `libyaml-cpp-dev`, `libopencv-dev`, and
  `libgstreamer-plugins-base1.0-dev` in the base image so the Edge AI SDK and
  customer programs build on the board out of the box.

#### `libdrm` dependency conflict resolved

- Align `libdrm` to the Debian `2.4.124-2` packages (replacing a non-standard
  `2.4.127` build), so installing `*-dev` packages (e.g. GStreamer plugins-base
  dev) no longer hits held / broken dependency conflicts.

### Flashing & Build

- `bin/mo-62a-flash.sh` now builds `edgeai-cpp` and installs the full C/C++ SDK
  (binary, headers, libraries, CMake package, examples) into the target rootfs
  during imaging, via a qemu-aarch64 chroot build, and passes the selected rootfs
  tarball through so the build matches the runtime ABI.
- Remove a stale duplicate `apps_cpp` source tree from the rootfs overlay; the
  canonical C/C++ demo source now ships only under the SDK examples.

## v1.0.5 — 2026-05-07

### Kernel & Device Tree

#### 40-Pin Expansion Header — Peripheral Mode DT Overlay

- Add `k3-am62a7-mo-62a-exp-periph.dtso` — a new DT overlay that enables optional
  peripheral functions on the 40-pin expansion header (J9):
  - **Pins 3/5**: WKUP\_I2C0 SDA/SCL (`/dev/i2c-0`)
  - **Pins 8/10**: UART5 TXD/RXD (`/dev/ttyS3`)
  - **Pins 19/21/23/24/26**: SPI0 D0/D1/CLK/CS0/CS1
  - **Pins 32/33**: EHRPWM0\_B/A (`/sys/class/pwm/pwmchip0` ch1/ch0)
  - **Pins 12/35/38/40**: MCASP2 ACLKX/AFSX/AXR0/AXR1
- The overlay provides reduced pinctrl groups for `mcu_gpio0` and `main_gpio1`
  (excluding pins claimed by the above peripherals) to prevent GPIO controller
  probe failures at boot.
- Pad 0x0174 (GPIO0\_91 / pin 35) is removed from `gpio0-default-pins` in the overlay
  to resolve a conflict between the SiI9022 HDMI bridge and MCASP2\_AFSX.
- Add `dtb-$(CONFIG_ARCH_K3) += k3-am62a7-mo-62a-exp-periph.dtbo` to the kernel DTS
  `Makefile`; the overlay is built automatically by `make linux-dtbs`.

#### 40-Pin SPI0 — spidev Nodes

- Add `spidev@0` (CS0, Pin 24) and `spidev@1` (CS1, Pin 26) child nodes under
  `main_spi0` in the overlay, using `compatible = "rohm,dh2228fv"` (modern kernels
  reject the plain `"spidev"` compatible string).
- `/dev/spidev0.0` is available after boot. Verified with an 8-byte hardware loopback
  test (Pin 19 MOSI ↔ Pin 21 MISO shorted) — TX and RX data identical.

#### 40-Pin Audio — Waveshare WM8960 Audio HAT Support

- Add `wm8960@1a` (`compatible = "wlf,wm8960"`) under `wkup_i2c0` in the overlay;
  the WM8960 driver probes successfully over WKUP\_I2C0 (pins 3/5, i2c-0).
- Extend `mcasp2` overlay node with `op-mode = IIS`, `tdm-slots = 2`, `serial-dir`
  (AXR0 = RX microphone, AXR1 = TX headphone), and
  `system-clock-frequency = 24576000` (same technique used by the HDMI audio path to
  provide an integer BCLK divisor despite the SoC fck being 24 615 384 Hz).
- Add `sound-wm8960 simple-audio-card` at the root node via an `&{/}` overlay fragment;
  MCASP2 is I2S master, WM8960 is codec slave.
- Verified on hardware:
  - WM8960 probes at address 0x1a (`i2cdetect` shows `UU`)
  - ALSA card `WM8960-Sound` registered; 48 kHz stereo playback confirmed audible
  - 5-second capture (S32\_LE, onboard dual MEMS microphones) played back correctly

#### 40-Pin Expansion Header — Per-Peripheral DT Overlay Files

- Add five individual DT overlay files, each enabling exactly one peripheral group:
  - `k3-am62a7-mo-62a-exp-i2c0.dtso`: WKUP\_I2C0 only (pins 3/5); MCU GPIO0 pinctrl
    reduced to exclude `MCU_GPIO0_19/20`.
  - `k3-am62a7-mo-62a-exp-uart5.dtso`: UART5 only (pins 8/10); GPIO1 pinctrl reduced
    to exclude `GPIO1_24/25`.
  - `k3-am62a7-mo-62a-exp-spi0.dtso`: SPI0 only (pins 19/21/23/24/26) with two spidev
    nodes; GPIO1 pinctrl reduced to exclude `GPIO1_15–19`.
  - `k3-am62a7-mo-62a-exp-ehrpwm0.dtso`: EHRPWM0 only (pins 32/33); GPIO1 pinctrl
    reduced to exclude `GPIO1_13/14`.
  - `k3-am62a7-mo-62a-exp-audio.dtso`: WM8960 Audio HAT — combines WKUP\_I2C0 (codec
    I²C control, pins 3/5) with MCASP2 (I²S data, pins 12/35/38/40); MCU GPIO0,
    GPIO1, and `gpio0-default-pins` pinctrl groups each updated to exclude only the
    pads claimed by this overlay.
- Each overlay carries its own precisely-reduced GPIO pinctrl group so that only the
  pads of the activated peripheral are removed from the GPIO controllers, preventing
  probe failures regardless of which single overlay is loaded.
- Add five corresponding `dtb-$(CONFIG_ARCH_K3) += k3-am62a7-mo-62a-exp-<name>.dtbo`
  entries to the kernel DTS `Makefile`.

### U-Boot

#### LPDDR4 Dual-Chip Compatibility — Samsung 2 GB / Micron 4 GB Runtime Detection

- Rename R5 DDR parameter file from `lp4-4GB.dtsi` to `lp4-Samsung-2GB-1866MHz.dtsi`
  and align its contents with verified Samsung LPDDR4 timing parameters; update
  `k3-am62a7-r5-mo-62a.dts` to include the renamed file as the default DDR config.
- Add `lp4_micron_4gb.h` containing a complete Micron 4 GB CTL/PI/PHY register table
  (2 805 entries) for runtime DDR re-initialisation.
- Update the U-Boot memory node in `k3-am62a7-mo-62a.dts` (A53 side) to declare
  2 GB (Samsung default); actual size is patched at runtime via FDT fixup.
- Remove unused `SPI`/`NAND`/`I2C` Kconfig symbols from `am62ax_mo_62a_r5_defconfig`.
- Implement runtime manufacturer detection in `k3-ddrss.c` (R5 SPL / tiboot3.bin):
  - After Samsung 2 GB initial training, read LPDDR4 MR5 (vendor ID) via the
    Cadence DDR driver `getmmrregister` interface.
  - **MR5 = 0x01 (Samsung)**: no further action; single 2 GB bank retained.
  - **MR5 = 0xFF (Micron)**: perform a full DDR subsystem reset using direct PSC
    `MDCTL` register writes (TI-SCI cannot power-cycle shared clock domains at
    SPL stage), re-initialise with Micron 4 GB CTL/PI/PHY tables, and set
    `ddr_bank1_size = 0x80000000` at base `0x880000000`.
- Extend `spl_perform_fixups()` in `board/ti/am62ax/evm.c` to call
  `k3_ddrss_fdt_fixup_memory()` for `CONFIG_K3_DDRSS` builds (without inline ECC),
  propagating the actual memory layout through the full FDT chain:
  - R5 SPL patches tispl FDT with the real bank layout
  - A53 SPL reads the updated FDT and patches the U-Boot FDT
  - A53 U-Boot displays `DRAM:  2 GiB (total 4 GiB)` and patches the Linux DTB
  - Linux kernel sees the full 4 GiB via the `/memory` node
- Samsung 2 GB boards are unaffected: MR5 = 0x01 matches no compat entry,
  `bank1_size` remains zero, FDT fixup writes only the single 2 GB bank.
  The same SD card image boots correctly on both hardware variants.
- Verified on hardware:
  - Samsung 2 GB: `DRAM:  2 GiB`, Linux `/proc/meminfo` ≈ 2 GB
  - Micron 4 GB:  `DRAM:  2 GiB (total 4 GiB)`, Linux `/proc/meminfo` ≈ 4 GB

### Boot Configuration

- `bin/extlinux/extlinux.conf` restructured with 7 labels:
  - `microSD` — all-GPIO mode (**default**)
  - `microSD-i2c0` — WKUP\_I2C0 (pins 3/5)
  - `microSD-uart5` — UART5 (pins 8/10)
  - `microSD-spi0` — SPI0 with two spidev CS (pins 19/21/23/24/26)
  - `microSD-ehrpwm0` — EHRPWM0 PWM outputs (pins 32/33)
  - `microSD-audio` — WM8960 Audio HAT via WKUP\_I2C0 + MCASP2 (pins 3/5/12/35/38/40)
  - `microSD-periph` — all special functions enabled simultaneously
- **Default boot label is `microSD` (all-GPIO mode).** Select a peripheral mode by
  editing the `default` line in extlinux.conf or by interrupting U-Boot at boot and
  choosing a label interactively.

---

## v1.0.4 — 2026-04-28

### Kernel & Device Tree

#### SiI9022A HDMI Bridge — Supply Rail Registration (k3-am62a7-mo-62a.dts)
- Add `vdd_1v2_hdmi: regulator-7` fixed-regulator node (1.2 V, always-on) to represent
  the TLV75512PDQN (U8) LDO that powers the SiI9022ACNU CVCC12 rail.
- Add `iovcc-supply = <&vcc_3v3_sys>` and `cvcc12-supply = <&vdd_1v2_hdmi>` to the
  `sii9022` bridge node so the driver can locate both supply rails.
  Previously the driver printed `supply iovcc not found, using dummy regulator` and
  `supply cvcc12 not found, using dummy regulator` at every boot; both warnings are
  now eliminated.

#### omap-mailbox — Disable Unused Cluster 3 (k3-am62a7-mo-62a.dts)
- Disable `mailbox0_cluster3` in DTS (`status = "disabled"`).
  AM62A7 has four mailbox hardware instances but only three remoteproc consumers
  (C7x DSP + MCU-R5F + MAIN-R5F); cluster 3 has no registered mbox devices, causing
  the kernel to print `omap mailbox: no available mbox devices found` at every boot.

#### Device-Tree Dependency Cycles — Reduce Log Verbosity (drivers/base/core.c)
- Change the `Fixed dependency cycle(s) with` message from `pr_info` to `pr_debug`.
  The kernel auto-resolves these cycles; the messages are informational only and
  are no longer visible at the default console log level.

#### S1 Power Button — TPS6593-Q1 PMIC Driver (tps6594-core.c)
- Configure `NPWRON_SEL` (bits [7:6] of `NPWRON_CONF` register, address 0x3C) to
  button mode (`01`) on PMIC probe when the `system-power-controller` DT property is
  set. In button mode the NPWRON pin generates an interrupt on S1 press/release rather
  than acting as a simple enable signal.
- Register an IRQ handler for `TPS6594_IRQ_NPWRON_START` that reports `KEY_POWER`
  press and release events to a new `tps6594-pwrbutton` input device. Release is
  detected by polling `GPIO_IN_2` (NPWRON input state bit) every 50 ms via a
  `delayed_work` since the PMIC only generates a single start-edge interrupt per press.
- Remove `TPS6594_IRQ_NPWRON_START` from `tps6594_pfsm_resources[]` to prevent an
  IRQ ownership conflict (`-EBUSY`) with the new handler.
- Add a `register_reboot_notifier` callback that switches `NPWRON_SEL` back to ENABLE
  mode (`00`) on `SYS_POWER_OFF`. This runs during `kernel_shutdown_prepare()` while
  I²C is still operational, restoring the default power-on behaviour so that a short
  S1 press can restart the system after a clean soft poweroff.

### Rootfs

#### Wi-Fi Regulatory Database — Switch to Upstream-Signed Version
- Remove the Debian-signed `regulatory.db` and its detached signature from the rootfs
  tarball (`/lib/firmware/regulatory.db-debian` and `regulatory.db.p7s-debian`).
  The kernel is built with the `sforshee` and `wens` X.509 certificates only; the
  Debian-signed variant cannot be verified against these keys and caused the boot message
  `cfg80211: loaded regulatory.db is malformed or signature is missing/invalid`.
- The `update-alternatives` auto mode selects the upstream-signed `regulatory.db` (symlink
  `/lib/firmware/regulatory.db → regulatory.db-upstream`) as the active database.
  This is the version the kernel can verify; the warning is eliminated.
- Subsequent `apt upgrade` of the `wireless-regdb` package will update
  `regulatory.db-upstream` in-place; the upstream variant remains selected in auto mode
  and the kernel continues to verify it successfully.

#### S1 Power Button Timing Daemon
- Add `board-support/extra-applications/s1-powerkey/` Python daemon that mirrors
  standard Ubuntu laptop power-button behaviour:
  - **Press** → `xfce4-session-logout` XFCE shutdown dialog appears immediately
    (non-blocking, fired via a daemon thread while the button is still held)
  - **Release before 5 s** → poweroff timer cancelled; dialog stays open for user
    interaction
  - **Hold ≥ 5 s** → `systemctl poweroff` (dialog bypassed)
  - No XFCE session (login screen): dialog step silently skipped; 5 s poweroff
    timer still fires normally
- Add `board-support/rootfs-overlay/etc/systemd/system/s1-powerkey.service`: simple
  service with `Restart=always`; `WantedBy=multi-user.target` only — no
  `After=graphical.target` / `Wants=graphical.target` to avoid a systemd ordering
  cycle that silently prevented the service from starting at boot.
- Add `multi-user.target.wants/s1-powerkey.service` symlink for auto-start.
- Add `board-support/rootfs-overlay/etc/systemd/logind.conf.d/s1-powerkey.conf`:
  sets `HandlePowerKey=ignore` and `HandlePowerKeyLongPress=ignore` so logind does
  not consume `KEY_POWER` events before the daemon can handle them.

---

## v1.0.3 — 2026-04-21

### Rootfs

#### Image Size Reduction (~1.76 GB uncompressed / ~400 MB compressed)
- Remove Thunderbird and LibreOffice (including all l10n and UI packages) via
  `apt-get remove --purge` + autoremove; frees ~487 MB.
- Slim Noto font collection: remove `fonts-noto-extra`, `fonts-noto-cjk-extra`,
  `fonts-noto-ui-extra`, `fonts-noto-unhinted`, and the `fonts-noto` meta-package;
  retain `fonts-noto-core`, `fonts-noto-cjk`, `fonts-noto-mono`; frees ~657 MB.
- Remove third-party vendor firmware not used by MO-62A hardware: Qualcomm
  (ath10k/ath11k/ath12k/qca), Intel (iwlwifi/intel), MediaTek, Broadcom (brcm),
  Marvell (mrvl/libertas/mwl8k), Cypress, Atheros (ath6k/ar3k), Wilocity, Ralink,
  and miscellaneous single-file blobs; retain TI (ti-connectivity/ti-ipc), Realtek
  (rtw89/rtw88/rtlwifi/rtl_bt/rtlbt/rtl_nic/realtek), AM62A VPU
  (cnm/vpu_d.bin/vpu_p.bin), and the wireless regulatory database; frees ~306 MB.
- Slim locale data: remove all locales except zh_CN, zh_TW, zh_HK, en, en_US,
  en_GB, and locale.alias; frees ~307 MB.
- Pre-install `memtester` and `mbw` for LPDDR4 integrity and bandwidth testing.

### Tools

#### mo62a-tester — Storage Test Suite
- Add `tests/test_storage.py` with a new `Storage` test category (`cat_storage`):
  - `LpddrMemtesterTest`: runs `sudo memtester 32M 1` (19 test patterns); size
    reduced from 128 M to 32 M to keep runtime under 60 s (~52 s typical).
  - `LpddrBandwidthTest`: runs `mbw -n 3 256`, parses MEMCPY AVG result, reports
    MiB/s and fails if below 1000 MiB/s threshold (~1478 MiB/s typical on AM62A).
  - `SdSpeedModeTest`: reads negotiated SD card speed mode and clock frequency from
    `/sys/kernel/debug/mmc1/ios` (sudo); falls back to `journalctl` grep on failure.
  - `SdReadSpeedTest`: drops page cache then measures sequential read speed with
    `dd if=/dev/mmcblk1 of=/dev/null bs=4M count=50`; threshold ≥ 15 MB/s.
  - `SdWriteSpeedTest`: measures sequential write speed with
    `dd if=/dev/zero of=/tmp/sd_write_test bs=4M count=50 oflag=dsync`;
    threshold ≥ 5 MB/s.
- Register `cat_storage` in `gui/page_select.py` `TEST_CATEGORIES`.
- Add all Storage category strings to `gui/i18n.py` (EN + ZH).

---

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

#### Pinmux Dead Code Removal
- Remove orphaned `main_ehrpwm1_pins_default` pinctrl group from
  `k3-am62a7-mo-62a-pinmux.dtsi`. The group claimed pad 0x019c
  (`MCASP0_AXR1`, ball B18) in EHRPWM1_A mux mode but was never referenced
  by any DTS node, leaving `gpio1_pins_default` (which also claims pad 0x019c
  in GPIO mux mode) as the only active consumer. Removed to eliminate the
  latent conflict and clean up dead entries.
- Fix annotation on GPIO1_9 (pad 0x019c) in `gpio1_pins_default`: corrected
  the signal name to `/* (B18) MCASP0_AXR1.GPIO1_9 */`.

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

#### nginx — Missing Log Directory Fix
- Add `usr/lib/tmpfiles.d/nginx.conf` to rootfs overlay: instructs
  `systemd-tmpfiles` to create `/var/log/nginx/` (owner `www-data:adm`, mode
  0755) at boot, fixing the nginx startup failure caused by the missing
  directory in the base rootfs image.

### Tools

#### mo-version
- Add `board-support/extra-applications/mo-version/`: a minimal C utility
  installed to `/usr/local/bin/mo-version` that prints the board BSP version
  and build date baked in at compile time:
  ```
  MO-62A v1.0.2
  Built:  2026-04-17
  ```
  The flash script passes `VERSION` and `BUILD_DATE` as make variables so the
  binary always reflects the image that was flashed.

### Documentation

#### QuickStart — PWM Pin Table Correction
- Remove incorrect PWM sysfs example from §8.4 that referenced `pwmchip0`
  channel 0 for expansion-header Pin 32 (`BCM GPIO12`). `pwmchip0` is the fan
  PWM controller, not the expansion header; the example would silently affect
  the fan instead of the expansion header. No corrected example is provided
  because no PWM output is currently routed to the expansion header pins.

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
