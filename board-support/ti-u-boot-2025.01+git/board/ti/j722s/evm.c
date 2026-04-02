// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for J722S platforms
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#include <asm/arch/hardware.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/uclass-internal.h>
#include <dm/root.h>
#include <env.h>
#include <fdt_support.h>
#include <spl.h>
#include <asm/arch/k3-ddr.h>
#include "../common/fdt_ops.h"

ofnode cadence_qspi_get_subnode(struct udevice *dev)
{
	if (IS_ENABLED(CONFIG_SPL_BUILD) &&
	    IS_ENABLED(CONFIG_TARGET_J721S2_R5_EVM)) {
		if (spl_boot_device() == BOOT_DEVICE_SPINAND)
			return ofnode_by_compatible(dev_ofnode(dev), "spi-nand");
	}

	return dev_read_first_subnode(dev);
}

/* Enables the spi-nand dts node, if onboard mux is set to spinand */
static void __maybe_unused detect_enable_spinand(void *blob)
{
	if (IS_ENABLED(CONFIG_DM_GPIO) && IS_ENABLED(CONFIG_OF_LIBFDT)) {
		struct gpio_desc desc = {0};
		char *ospi_mux_sel_gpio = "gpio@23_1";
		int nand_offset, nor_offset;

		if (dm_gpio_lookup_name(ospi_mux_sel_gpio, &desc))
			return;

		if (dm_gpio_request(&desc, ospi_mux_sel_gpio))
			return;

		if (dm_gpio_set_dir_flags(&desc, GPIOD_IS_IN))
			return;

		nand_offset = fdt_node_offset_by_compatible(blob, -1, "spi-nand");
		if (nand_offset < 0)
			return;

		nor_offset = fdt_node_offset_by_compatible(blob,
							   fdt_parent_offset(blob, nand_offset),
							   "jedec,spi-nor");

		if (dm_gpio_get_value(&desc)) {
			fdt_status_okay(blob, nand_offset);
			fdt_del_node(blob, nor_offset);
		} else {
			fdt_del_node(blob, nand_offset);
		}
	}
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	detect_enable_spinand(blob);

	return 0;
}
#endif

int board_init(void)
{
	return 0;
}

#if defined(CONFIG_XPL_BUILD)
void spl_perform_fixups(struct spl_image_info *spl_image)
{
	if (IS_ENABLED(CONFIG_K3_DDRSS)) {
		if (IS_ENABLED(CONFIG_K3_INLINE_ECC))
			fixup_ddr_driver_for_ecc(spl_image);
	} else {
		fixup_memory_node(spl_image);
	}

	detect_enable_spinand(spl_image->fdt_addr);
}
#endif

#if IS_ENABLED(CONFIG_BOARD_LATE_INIT)
int board_late_init(void)
{
	ti_set_fdt_env(NULL, NULL);
	return 0;
}
#endif

#if IS_ENABLED(CONFIG_SPL_BOARD_INIT)
void spl_board_init(void)
{
	u32 val;

	/* We have 32k crystal, so lets enable it */
	val = readl(MCU_CTRL_LFXOSC_CTRL);
	val &= ~(MCU_CTRL_LFXOSC_32K_DISABLE_VAL);
	writel(val, MCU_CTRL_LFXOSC_CTRL);
	/* Add any TRIM needed for the crystal here.. */
	/* Make sure to mux up to take the SoC 32k from the crystal */
	writel(MCU_CTRL_DEVICE_CLKOUT_LFOSC_SELECT_VAL,
	       MCU_CTRL_DEVICE_CLKOUT_32K_CTRL);
}
#endif
