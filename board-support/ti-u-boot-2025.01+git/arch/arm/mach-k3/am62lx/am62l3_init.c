// SPDX-License-Identifier: GPL-2.0
/*
 * AM62Lx: SoC specific initialization
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <hang.h>
#include <spl.h>

#include "../common.h"

void board_init_f(ulong dummy)
{
	int ret;

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();
	spl_enable_cache();
	debug("am62lx_init: %s done\n", __func__);
}

u32 spl_boot_device(void)
{
	return get_boot_device();
}

u32 spl_mmc_boot_mode(struct mmc *mmc, const u32 boot_device)
{
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);
	u32 bootmode = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
		MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;
	u32 bootmode_cfg = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK) >>
		MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BOOT_DEVICE_EMMC:
		if (IS_ENABLED(CONFIG_SUPPORT_EMMC_BOOT))
			return MMCSD_MODE_EMMCBOOT;
		if (IS_ENABLED(CONFIG_SPL_FS_FAT) || IS_ENABLED(CONFIG_SPL_FS_EXT4))
			return MMCSD_MODE_FS;
		return MMCSD_MODE_EMMCBOOT;
	case BOOT_DEVICE_MMC:
		if (bootmode_cfg & MAIN_DEVSTAT_PRIMARY_MMC_FS_RAW_MASK)
			return MMCSD_MODE_RAW;
	default:
		return MMCSD_MODE_FS;
	}
}

