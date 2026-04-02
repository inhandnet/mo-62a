/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * AM62Lx SoC definitions, structures etc.
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __ASM_ARCH_AM62L_HARDWARE_H
#define __ASM_ARCH_AM62L_HARDWARE_H

#include <config.h>
#ifndef __ASSEMBLY__
#include <linux/bitops.h>
#endif

#define CTRL_MMR0_BASE				0x00100000
#define WKUP_CTRL_MMR0_BASE			0x43010000

#define CTRLMMR_MAIN_DEVSTAT			(WKUP_CTRL_MMR0_BASE + 0x30)
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK	GENMASK(6, 3)
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT	3
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK	GENMASK(9, 7)
#define MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT	7
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK	GENMASK(12, 10)
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT	10
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK	BIT(13)
#define MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT	13

/* Primary Bootmode MMC Config macros */
#define MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK	BIT(2)
#define MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT	2
#define MAIN_DEVSTAT_PRIMARY_MMC_FS_RAW_MASK	BIT(0)
#define MAIN_DEVSTAT_PRIMARY_MMC_FS_RAW_SHIFT	0

/* Primary Bootmode USB Config macros */
#define MAIN_DEVSTAT_PRIMARY_USB_MODE_SHIFT	1
#define MAIN_DEVSTAT_PRIMARY_USB_MODE_MASK	BIT(1)

/* Backup Bootmode USB Config macros */
#define MAIN_DEVSTAT_BACKUP_USB_MODE_MASK	BIT(0)

/* address boot rom leaves its boot structure */
#define K3_BOOT_PARAM_TABLE_INDEX_OCRAM		0x70816e70

static const u32 put_core_ids[] = {};

#endif /* __ASM_ARCH_AM62L_HARDWARE_H */
