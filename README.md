# MO-62A SDK

MO-62A single-board computer SDK, powered by the TI AM62A7 platform, offering up to 2 TOPS AI performance.

---

## Table of Contents

- [1. Overview](#1-overview)
- [2. Environment Setup](#2-environment-setup)
  - [2.1 Host System Requirements](#21-host-system-requirements)
  - [2.2 Clone This Repository](#22-clone-this-repository)
  - [2.3 Initialize the Environment with setup.sh](#23-initialize-the-environment-with-setupsh)
  - [2.4 Toolchain](#24-toolchain)
- [3. U-Boot](#3-u-boot)
  - [3.1 Related Files](#31-related-files)
  - [3.2 Build U-Boot](#32-build-u-boot)
  - [3.3 Stage Build Artifacts](#33-stage-build-artifacts)
  - [3.4 Build Output](#34-build-output)
- [4. Linux Kernel](#4-linux-kernel)
  - [4.1 Related Files](#41-related-files)
  - [4.2 Build DTBs](#42-build-dtbs)
  - [4.3 Build Kernel](#43-build-kernel)
  - [4.4 Stage Build Artifacts](#44-stage-build-artifacts)
  - [4.5 Build Output](#45-build-output)
- [5. Flashing the SD Card](#5-flashing-the-sd-card)
  - [5.1 Prerequisites](#51-prerequisites)
  - [5.2 Launch the Flash Tool](#52-launch-the-flash-tool)
  - [5.3 Online Flashing (Write Directly to SD Card)](#53-online-flashing-write-directly-to-sd-card)
  - [5.4 Offline Image Creation (balenaEtcher)](#54-offline-image-creation-balenaetcher)
- [6. Partition Layout](#6-partition-layout)
  - [6.1 BOOT Partition Contents](#boot-partition-contents)
  - [6.2 Boot Configuration](#boot-configuration)

---

## 1. Overview

MO-62A is a single-board computer based on the TI AM62A7 processor, designed for edge AI inference, machine vision, and industrial control applications. The AM62A7 integrates a quad-core Arm Cortex-A53 (up to 1.4 GHz), a single Cortex-R5F real-time processor, and a dedicated MMA (Matrix Multiplication Accelerator), delivering up to 2 TOPS of AI inference performance.

This SDK is built on top of **TI Processor SDK Linux RT EdgeAI AM62A EVM 11.01.07.05** and provides full hardware customization support for the MO-62A board. It includes the following components:

- **U-Boot**: Bootloader customized for MO-62A, including R5 SPL and A53 U-Boot
- **Linux Kernel**: Kernel and device trees customized for MO-62A, based on TI Linux RT kernel
- **Build System**: Unified Makefile-based build and staging workflow
- **Flash Tool**: Supports direct SD card writing and offline balenaEtcher image creation

SDK directory structure overview:

```
mo-62a/
├── board-support/          # U-Boot, Kernel sources and prebuilt images
│   ├── built-images/       # Staged build artifacts (generated after staging)
│   ├── prebuilt-images/    # TI prebuilt firmware (bl31, bl32, ti-dm, etc.)
│   └── extra-applications/ # Board-specific extra applications
├── bin/                    # Helper scripts and flash tool
│   ├── mo-62a-flash.sh     # Unified flash tool
│   └── extlinux/           # U-Boot extlinux boot configuration
├── filesystem/             # rootfs tarball directory
├── linux-devkit/           # AArch64 cross-compilation toolchain
├── k3r5-devkit/            # ARMv7 R5 cross-compilation toolchain
├── makerules/              # Per-component Makefile rules
├── Makefile                # Top-level Makefile
├── Rules.make              # Global path and platform configuration
└── setup.sh                # Development host initialization script
```

---

## 2. Environment Setup

### 2.1 Host System Requirements

The following host environment has been verified for building this SDK:

| Item | Requirement |
|------|-------------|
| OS | Ubuntu 22.04 LTS (x86_64) |
| Kernel | 6.8.0 or later |
| Architecture | x86_64 |

Required host packages:

```bash
sudo apt-get install \
  xinetd tftpd nfs-kernel-server minicom \
  build-essential libncurses5-dev autoconf automake \
  dos2unix screen lrzsz lzop flex libssl-dev \
  u-boot-tools make git parted dosfstools e2fsprogs \
  pv xz-utils zip wget curl
```

### 2.2 Clone This Repository

Clone the MO-62A SDK to any directory on your host machine:

```bash
git clone https://github.com/inhandnet/mo-62a.git
cd mo-62a
```

### 2.3 Initialize the Environment with setup.sh

Run `setup.sh` from the SDK root to initialize the development host:

```bash
./setup.sh
```

The script performs the following steps:

1. **Verifies** the host OS (Ubuntu 22.04 LTS)
2. **Adds** the current user to the `dialout` group (required for serial port access)
3. **Installs** required host packages — press `Y` to install, `n` to skip if already installed
4. **Writes** `TI_SDK_PATH` to `~/.bashrc` pointing to the cloned repository root, so the variable persists across terminal sessions
5. **Creates** a symlink at `/opt/ti-processor-sdk-linux-rt-edgeai-am62a-evm-11.01.07.05` pointing to the cloned repository, required because the cross-compilation toolchain binaries have a hardcoded ELF interpreter path at that location

Example output:

```
-------------------------------------------------------------------------------
MO-62A SDK setup script
SDK root: /home/user/mo-62a
-------------------------------------------------------------------------------

Verifying Linux host distribution
Ubuntu 22.04 LTS is being used, continuing..

User 'user' is already in the 'dialout' group.

Do you wish to install required host packages? (Y/n) n
Host package installation skipped.

TI_SDK_PATH is already set correctly in /home/user/.bashrc:
  export TI_SDK_PATH="/home/user/mo-62a"
TI_SDK_PATH is now set to: /home/user/mo-62a

Creating toolchain symlink...
  /opt/ti-processor-sdk-linux-rt-edgeai-am62a-evm-11.01.07.05 -> /home/user/mo-62a

-------------------------------------------------------------------------------
MO-62A SDK setup completed!
You can now build the SDK from: /home/user/mo-62a
-------------------------------------------------------------------------------
```

> **Note:** After `setup.sh` completes, `TI_SDK_PATH` is active in the current shell and will be automatically set in all future terminal sessions via `~/.bashrc`. Running `setup.sh` again is safe — it will update the symlink and the `TI_SDK_PATH` entry without creating duplicates.

### 2.4 Toolchain

Two cross-compilation toolchains are included in the SDK:

**AArch64 toolchain** — used for Linux kernel and A53 U-Boot:

| Item | Value |
|------|-------|
| Path | `linux-devkit/sysroots/x86_64-arago-linux/usr/bin/aarch64-oe-linux/` |
| Prefix | `aarch64-oe-linux-` |
| GCC version | 13.4.0 |
| Target sysroot | `linux-devkit/sysroots/aarch64-oe-linux/` |

**ARMv7 R5 toolchain** — used for R5 SPL (tiboot3):

| Item | Value |
|------|-------|
| Path | `k3r5-devkit/sysroots/x86_64-arago-linux/usr/bin/arm-oe-eabi/` |
| Prefix | `arm-oe-eabi-` |
| GCC version | 13.4.0 |
| Target sysroot | `k3r5-devkit/sysroots/armv7at2hf-vfp-oe-eabi/` |

To verify both toolchains are available (run after `setup.sh` has completed):

```bash
# AArch64
linux-devkit/sysroots/x86_64-arago-linux/usr/bin/aarch64-oe-linux/aarch64-oe-linux-gcc --version

# ARMv7 R5
k3r5-devkit/sysroots/x86_64-arago-linux/usr/bin/arm-oe-eabi/arm-oe-eabi-gcc --version
```

Expected output:

```
aarch64-oe-linux-gcc (GCC) 13.4.0
Copyright (C) 2023 Free Software Foundation, Inc.

arm-oe-eabi-gcc (GCC) 13.4.0
Copyright (C) 2023 Free Software Foundation, Inc.
```

> **Note:** The toolchain binaries have a hardcoded ELF interpreter path pointing to `/opt/ti-processor-sdk-linux-rt-edgeai-am62a-evm-11.01.07.05/`. The symlink created by `setup.sh` satisfies this requirement regardless of where the repository is cloned. The toolchains are invoked automatically via `Rules.make` and do not require manual PATH export.

---

## 3. U-Boot

### 3.1 Related Files

MO-62A specific files added to the U-Boot source tree:

| File | Description |
|------|-------------|
| `board-support/ti-u-boot-2025.01+git/configs/am62ax_mo_62a_a53_defconfig` | A53 U-Boot defconfig |
| `board-support/ti-u-boot-2025.01+git/configs/am62ax_mo_62a_r5_defconfig` | R5 SPL defconfig |
| `board-support/ti-u-boot-2025.01+git/dts/upstream/src/arm64/ti/k3-am62a7-mo-62a.dts` | A53 main device tree |
| `board-support/ti-u-boot-2025.01+git/dts/upstream/src/arm64/ti/k3-am62a7-mo-62a-pinmux.dtsi` | Pin mux configuration |
| `board-support/ti-u-boot-2025.01+git/arch/arm/dts/k3-am62a7-r5-mo-62a.dts` | R5 SPL device tree |
| `board-support/ti-u-boot-2025.01+git/arch/arm/dts/k3-am62a7-mo-62a-u-boot.dtsi` | U-Boot specific DT additions |
| `board-support/ti-u-boot-2025.01+git/arch/arm/dts/k3-am62a7-mo-62a-binman.dtsi` | Binman packaging configuration |
| `board-support/ti-u-boot-2025.01+git/arch/arm/dts/k3-am62a7-mo-62a-lp4-4GB.dtsi` | LPDDR4 4GB memory configuration |

Prebuilt firmware required during the build (from TI, not modified):

| File | Description |
|------|-------------|
| `board-support/prebuilt-images/am62a-evm/bl31.bin` | Arm Trusted Firmware (TF-A) |
| `board-support/prebuilt-images/am62a-evm/bl32.bin` | OP-TEE OS |
| `board-support/prebuilt-images/am62a-evm/ti-dm/am62axx/dm_edgeai_mcu1_0_release_strip.out` | TI Device Manager firmware |

### 3.2 Build U-Boot

U-Boot is built in two separate passes — R5 SPL first, then A53 U-Boot. Both are invoked via the top-level `make` from the SDK root.

**Build R5 SPL (tiboot3):**

```bash
make u-boot-r5
```

This configures the R5 build with `am62ax_mo_62a_r5_defconfig` and compiles using the ARMv7 R5 toolchain (`arm-oe-eabi-`). The output is placed in `board-support/u-boot-build/r5/`.

**Build A53 U-Boot:**

```bash
make u-boot-a53
```

This configures the A53 build with `am62ax_mo_62a_a53_defconfig` and compiles using the AArch64 toolchain (`aarch64-oe-linux-`), incorporating `bl31.bin`, `bl32.bin`, and the TI Device Manager firmware via binman. The output is placed in `board-support/u-boot-build/a53/`.

**Build both in one step:**

```bash
make u-boot
```

### 3.3 Stage Build Artifacts

After a successful build, copy the output binaries to `board-support/built-images/`:

```bash
make u-boot_stage
```

### 3.4 Build Output

After staging, the following files are available in `board-support/built-images/`:

| File | Size | Source | Description |
|------|------|--------|-------------|
| `tiboot3-am62ax-gp-mo-62a.bin` | ~318 KB | R5 build | R5 SPL for GP (General Purpose) devices |
| `tiboot3-am62ax-hs-fs-mo-62a.bin` | ~320 KB | R5 build | R5 SPL for HS-FS (High Security, Field Securable) devices |
| `tiboot3-am62ax-hs-mo-62a.bin` | ~320 KB | R5 build | R5 SPL for HS (High Security) devices |
| `tiboot3.bin` | ~320 KB | R5 build | Default tiboot3 (HS-FS, used by flash tool) |
| `tispl.bin` | ~1.7 MB | A53 build | TI SPL — loads OP-TEE, TF-A, and A53 U-Boot |
| `u-boot.img` | ~1.2 MB | A53 build | A53 U-Boot FIT image |

> **Note:** The appropriate `tiboot3-*.bin` variant depends on the device security state. Most production MO-62A boards ship as HS-FS, which uses `tiboot3-am62ax-hs-fs-mo-62a.bin`. The flash tool uses `tiboot3.bin` (a copy of the HS-FS variant) by default.

---

## 4. Linux Kernel

### 4.1 Related Files

MO-62A specific files added to the Linux kernel source tree:

| File | Description |
|------|-------------|
| `board-support/ti-linux-kernel-6.12.35+git-ti-rt/arch/arm64/configs/am62ax_mo_62a_defconfig` | MO-62A base kernel defconfig |
| `board-support/ti-linux-kernel-6.12.35+git-ti-rt/arch/arm64/boot/dts/ti/k3-am62a7-mo-62a.dts` | MO-62A main device tree |
| `board-support/ti-linux-kernel-6.12.35+git-ti-rt/arch/arm64/boot/dts/ti/k3-am62a7-mo-62a-pinmux.dtsi` | Pin mux configuration |

The following kernel config fragments are applied on top of the defconfig during the build:

| Fragment | Location | Purpose |
|----------|----------|---------|
| `ti_arm64_prune.config` | `kernel/configs/` | Removes non-TI ARM64 platform support to reduce build size |
| `ti_rt.config` | `kernel/configs/` | Enables PREEMPT_RT real-time kernel patches |

### 4.2 Build DTBs

Build all device tree blobs (62 DTBs and DTBOs in total):

```bash
make linux-dtbs
```

This configures the kernel with `am62ax_mo_62a_defconfig` + `ti_arm64_prune.config` + `ti_rt.config`, then builds all DTBs matching the prefix patterns defined in `Rules.make`:

```
ti/k3-am62a7  ti/k3-fpdlink  ti/k3-am62x-sk  ti/k3-v3link
```

Output is placed in `board-support/ti-linux-kernel-6.12.35+git-ti-rt/arch/arm64/boot/dts/`.

### 4.3 Build Kernel

Build the kernel image, modules, and FitImage:

```bash
make linux
```

This performs the following steps in order:

1. Builds DTBs (calls `linux-dtbs` automatically)
2. Builds U-Boot (calls `u-boot` automatically, required for FitImage signing key)
3. Compiles `Image` and `Image.gz`
4. Compiles kernel modules
5. Packages a signed `fitImage` using `mkimage` with the U-Boot signing key and the ITS template from `board-support/prebuilt-images/am62a-evm/`
6. Rebuilds U-Boot A53 (binman) to embed the MO-62A DTB into `tispl.bin`

### 4.4 Stage Build Artifacts

Copy kernel and DTB outputs to `board-support/built-images/`:

```bash
make linux_stage
```

### 4.5 Build Output

After staging, the following files are added to `board-support/built-images/`:

| File | Size | Description |
|------|------|-------------|
| `Image` | ~18 MB | Uncompressed kernel image |
| `Image.gz` | ~7.0 MB | Compressed kernel image |
| `fitImage` | ~7.3 MB | Signed FIT image (kernel + DTBs), used for verified boot |
| `dtb/ti/*.dtb` / `dtb/ti/*.dtbo` | — | 62 device tree blobs and overlays |

> **Note:** `fitImage` is signed with the `custMpk` key from the U-Boot source tree. The signing step also triggers a rebuild of `tispl.bin` so it embeds the MO-62A DTB (`k3-am62a7-mo-62a.dtb`) in the A53 SPL. This means `make linux` will always update both `fitImage` and `tispl.bin` in `built-images/`.

---

## 5. Flashing the SD Card

The unified flash tool `bin/mo-62a-flash.sh` supports two output targets:

- **Online flashing** — write directly to a physical SD card
- **Offline image creation** — create a `.img` file for use with balenaEtcher

Both modes create identical partition layouts and content.

### 5.1 Prerequisites

Required host packages (installed by `setup.sh`):

```
parted  dosfstools  e2fsprogs  pv  xz-utils  zip  wget  curl
```

Ensure the following are ready before running the flash tool:

1. **Built images staged** — run `make u-boot_stage` and `make linux_stage` (or `make all_stage`) so that `board-support/built-images/` contains the required files
2. **Rootfs tarball present** — place a `.tar.xz` or `.tar.gz` rootfs tarball under `filesystem/`
3. **SD card inserted** (online mode only) — the tool automatically detects `/dev/sdX` and `/dev/mmcblkN` block devices

### 5.2 Launch the Flash Tool

The flash tool is interactive and must be run as root:

```bash
sudo bash bin/mo-62a-flash.sh
```

On launch, it prompts you to choose the output target:

```
=== MO-62A Flash Tool ===

Select output target:
  [1] Write directly to an SD card
  [2] Create offline image for balenaEtcher

Select target [1/2]:
```

### 5.3 Online Flashing (Write Directly to SD Card)

Select `[1]` for online mode. The tool will enumerate all detected block devices:

```
Detected candidate block devices:
  [1] /dev/sda  29.7G  SD Card Reader

Select target device number:
```

Then choose the operation mode:

```
Select operation mode:
  [1] full      - create partitions + format + copy BOOT + extract rootfs
  [2] partition - create partitions + format only
  [3] boot      - copy BOOT content only (strict checks, no repartition)
  [4] rootfs    - copy rootfs content only (strict checks, no repartition)

Select mode number (default 1):
```

| Mode | Description |
|------|-------------|
| `full` | Full write: wipe and repartition, format, copy boot files, extract rootfs, install kernel modules |
| `partition` | Partitioning only: wipe and recreate MBR, format partitions — no content written |
| `boot` | Overwrite BOOT partition only — re-flashes bootloader and kernel without touching rootfs |
| `rootfs` | Overwrite rootfs partition only — re-extracts the rootfs without touching the BOOT partition |

For a fresh SD card, select `full`. The tool will ask for confirmation (`Type YES to continue`), then proceed automatically:

1. Unmounts any existing partitions on the selected device
2. Wipes the old partition table
3. Creates MBR partition table: BOOT (FAT32, 256 MiB) + rootfs (ext4, remaining space)
4. Formats both partitions
5. Copies boot artifacts to the BOOT partition: `tiboot3.bin`, `tispl.bin`, `u-boot.img`, `Image`, DTBs, `extlinux/`
6. Asks which rootfs tarball to use (if multiple are present under `filesystem/`)
7. Extracts the rootfs tarball to the rootfs partition
8. Installs kernel modules (`make modules_install`) into the rootfs
9. Builds and installs any external applications from `board-support/extra-applications/`

Example session (full mode):

```
TARGET DEVICE: /dev/sda (29.7G)

MODE: full
This will ERASE ALL DATA on /dev/sda.

Destructive operation — please confirm.
Type YES to continue: YES

Checking mounts on /dev/sda ...
Wiping old partition table signatures...
Creating MBR partitions (BOOT FAT32(LBA) 256MiB + rootfs ext4 remaining)...
Formatting BOOT: /dev/sda1 (FAT32)
Formatting rootfs: /dev/sda2 (ext4)
Copying boot artifacts to BOOT partition...

Available rootfs tarballs:
  [1] debian-13.2-xfce-v6.12-arm64-2026-01-13-12gb.tar.xz

Select rootfs tarball number (or 'q' to abort): 1

Extracting rootfs to rootfs partition...
Installing kernel modules into rootfs: /tmp/mo-62a-rootfs.xxx/usr/lib/modules
...
Installed: rtk_hciattach

Done. BOOT and rootfs written successfully.
```

After flashing, insert the SD card into the MO-62A board and power on. Boot messages appear on the serial console at `ttyS2` (115200 8N1).

### 5.4 Offline Image Creation (balenaEtcher)

Select `[2]` for image mode. The tool prompts for image parameters:

```
Available rootfs tarballs:
  [1] debian-13.2-xfce-v6.12-arm64-2026-01-13-12gb.tar.xz

Select rootfs tarball number (or 'q' to abort): 1

Output directory (default: bin/out): /path/to/output
Output image base name (default: mo-62a): mo-62a
Image size (GiB, integer) (default: 8): 8
Compression (zip|xz|none) (default: zip): zip
```

| Parameter | Description |
|-----------|-------------|
| Output directory | Where to write the `.img` and checksum files |
| Image base name | Filename prefix; output will be `<name>.img` (and `<name>.img.zip` etc.) |
| Image size (GiB) | Total image size. Must be large enough to hold the rootfs; 8 GiB is sufficient for the default rootfs |
| Compression | `zip` (recommended for balenaEtcher), `xz` (smaller file, slower), `none` (no compression) |

The tool creates a sparse `.img` file, partitions and formats it via a loopback device, fills it with the same content as online mode, detaches the loop device, then compresses the result.

Output files:

```
/path/to/output/
├── mo-62a.img         # Raw disk image (8 GiB sparse file)
├── mo-62a.img.zip     # Compressed image for balenaEtcher (if compression=zip)
└── mo-62a.sha256      # SHA-256 checksums of all image files
```

To flash the image with balenaEtcher:

1. Open [balenaEtcher](https://etcher.balena.io/)
2. Click **Flash from file** and select `mo-62a.img.zip` (balenaEtcher accepts compressed images directly)
3. Select the target SD card
4. Click **Flash**

> **Note:** Image size must be at least as large as the expanded rootfs. The default `8` GiB is sufficient for the provided Debian rootfs (~6.9 GiB extracted). When using a custom rootfs, adjust the size accordingly.

---

## 6. Partition Layout

Both online flashing and offline image creation produce identical partition layouts:

| Partition | Number | Filesystem | Size | Label | Contents |
|-----------|--------|------------|------|-------|----------|
| BOOT | 1 | FAT32 (LBA) | 256 MiB | `BOOT` | Bootloader, kernel image, DTBs, extlinux config |
| rootfs | 2 | ext4 | Remaining | `rootfs` | Root filesystem, kernel modules |

Partition table type: **MBR** (Master Boot Record)

### BOOT Partition Contents

After flashing, the BOOT partition contains:

```
BOOT/
├── tiboot3.bin          # R5 SPL (first-stage bootloader)
├── tispl.bin            # TI SPL (OP-TEE + TF-A + A53 U-Boot)
├── u-boot.img           # A53 U-Boot FIT image
├── Image                # Linux kernel image
├── ti/                  # Device tree blobs directory
│   ├── k3-am62a7-mo-62a.dtb
│   └── ...              # Additional TI DTBs and DTBOs
└── extlinux/
    └── extlinux.conf    # U-Boot extlinux boot configuration
```

### Boot Configuration

U-Boot uses the extlinux boot flow to load the kernel. The default `extlinux/extlinux.conf`:

```
menu title mo-62a MicroSD (extlinux.conf)

timeout 30
default microSD

label microSD
  append console=ttyS2,115200n8 earlycon=ns16550a,mmio32,0x02800000 root=/dev/mmcblk1p2 rw rootfstype=ext4 rootwait
  kernel /Image
  fdt    /ti/k3-am62a7-mo-62a.dtb
  # fdtoverlays /overlays/<file>.dtbo
  # initrd /initrd.img
```

The kernel command line sets:
- Serial console: `ttyS2` at 115200 baud
- Root device: `/dev/mmcblk1p2` (partition 2 on the SD/MMC device)
- Root filesystem type: `ext4`

To apply a device tree overlay, uncomment the `fdtoverlays` line and specify the `.dtbo` file path relative to the BOOT partition root.
