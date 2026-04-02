// SPDX-License-Identifier: GPL-2.0+
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/am62l_spl.h>

static u32 get_backup_bootmedia(u32 devstat)
{
	u32 bootmode = (devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_MASK) >>
				MAIN_DEVSTAT_BACKUP_BOOTMODE_SHIFT;
	u32 bootmode_cfg = (devstat & MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_MASK) >>
				MAIN_DEVSTAT_BACKUP_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BACKUP_BOOT_DEVICE_UART:
		return BOOT_DEVICE_UART;

	case BACKUP_BOOT_DEVICE_MMC:
		if (bootmode_cfg)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BACKUP_BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BACKUP_BOOT_DEVICE_DFU:
		if (bootmode_cfg & MAIN_DEVSTAT_BACKUP_USB_MODE_MASK)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;
	};

	return BOOT_DEVICE_RAM;
}

static u32 get_primary_bootmedia(u32 devstat)
{
	u32 bootmode = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_MASK) >>
				MAIN_DEVSTAT_PRIMARY_BOOTMODE_SHIFT;
	u32 bootmode_cfg = (devstat & MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_MASK) >>
				MAIN_DEVSTAT_PRIMARY_BOOTMODE_CFG_SHIFT;

	switch (bootmode) {
	case BOOT_DEVICE_XSPI_FAST:
	case BOOT_DEVICE_XSPI:
	case BOOT_DEVICE_OSPI:
	case BOOT_DEVICE_QSPI:
	case BOOT_DEVICE_SPI:
		return BOOT_DEVICE_SPI;

	case BOOT_DEVICE_EMMC:
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_SPI_NAND:
		return BOOT_DEVICE_SPI_NAND;

	case BOOT_DEVICE_MMC:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_MMC_PORT_MASK) >>
				MAIN_DEVSTAT_PRIMARY_MMC_PORT_SHIFT)
			return BOOT_DEVICE_MMC2;
		return BOOT_DEVICE_MMC1;

	case BOOT_DEVICE_DFU:
		if ((bootmode_cfg & MAIN_DEVSTAT_PRIMARY_USB_MODE_MASK) >>
		    MAIN_DEVSTAT_PRIMARY_USB_MODE_SHIFT)
			return BOOT_DEVICE_USB;
		return BOOT_DEVICE_DFU;

	case BOOT_DEVICE_NOBOOT:
		return BOOT_DEVICE_RAM;
	}

	return bootmode;
}

u32 get_boot_device(void)
{
	u32 devstat = readl(CTRLMMR_MAIN_DEVSTAT);
	u32 bootmode = *(u32 *)(K3_BOOT_PARAM_TABLE_INDEX_OCRAM);

	if (bootmode == K3_PRIMARY_BOOTMODE)
		return get_primary_bootmedia(devstat);
	return get_backup_bootmedia(devstat);
}
