// SPDX-License-Identifier: GPL-2.0+
/*
 * Board specific initialization for AM62Ax platforms
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 *
 */

#include <asm/arch/hardware.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <env.h>
#include <fdt_support.h>
#include <spl.h>
#include <asm/arch/k3-ddr.h>

#include "../common/fdt_ops.h"

/*
 * MO-62A HW revision GPIO detection
 *
 * Three OSPI_CSn pins are wired as pull-down straps (R302/304/306, 10K to GND)
 * and selectively pulled up at manufacturing time to encode the LPDDR4 size:
 *
 *   HW_REV0 = GPIO0_11  (H21, OSPI0_CSn0, pad 0x02C)
 *   HW_REV1 = GPIO0_12  (G19, OSPI0_CSn1, pad 0x030)
 *   HW_REV2 = GPIO0_14  (G20, OSPI0_CSn3, pad 0x038)
 *
 *   REV0=1, REV1=0, REV2=0  →  2 GB  →  k3-am62a7-mo-62a-2gb.dtb
 *   REV0=0, REV1=1, REV2=0  →  4 GB  →  k3-am62a7-mo-62a-4gb.dtb
 *   REV0=0, REV1=0, REV2=1  →  8 GB  →  k3-am62a7-mo-62a-8gb.dtb
 *
 * K3 GPIO0 register layout (Davinci banked GPIO, bank 0):
 *   Base:           0x00600000
 *   Bank-0 offset:  + 0x10    (confirmed by da8xx_gpio.c)
 *   dir  register:  + 0x00    (bit=1 means input)
 *   in_data reg:    + 0x10
 */
#define MO62A_GPIO0_BASE       0x00600000UL
#define MO62A_GPIO0_BANK0_BASE (MO62A_GPIO0_BASE + 0x10)
#define MO62A_GPIO0_DIR        (MO62A_GPIO0_BANK0_BASE + 0x00)
#define MO62A_GPIO0_IN_DATA    (MO62A_GPIO0_BANK0_BASE + 0x10)

#define MO62A_HW_REV0_BIT 11  /* GPIO0_11 */
#define MO62A_HW_REV1_BIT 12  /* GPIO0_12 */
#define MO62A_HW_REV2_BIT 14  /* GPIO0_14 */

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
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
static int mo62a_get_lpddr4_gb(void)
{
	u32 dir, in;
	int rev0, rev1, rev2;

	/* Ensure direction is input (bit = 1 in Davinci GPIO dir register) */
	dir = readl(MO62A_GPIO0_DIR);
	dir |= BIT(MO62A_HW_REV0_BIT) | BIT(MO62A_HW_REV1_BIT) | BIT(MO62A_HW_REV2_BIT);
	writel(dir, MO62A_GPIO0_DIR);

	in   = readl(MO62A_GPIO0_IN_DATA);
	rev0 = (in >> MO62A_HW_REV0_BIT) & 1;
	rev1 = (in >> MO62A_HW_REV1_BIT) & 1;
	rev2 = (in >> MO62A_HW_REV2_BIT) & 1;

	if (rev0 && !rev1 && !rev2)
		return 2;
	if (!rev0 && rev1 && !rev2)
		return 4;
	if (!rev0 && !rev1 && rev2)
		return 8;

	/* All-zero = HW_REV resistors not yet soldered; treat as 4 GB */
	if (!rev0 && !rev1 && !rev2)
		printf("MO-62A: HW_REV straps unpopulated (all 0), defaulting to 4 GB\n");
	else
		printf("MO-62A: invalid HW_REV (REV0=%d REV1=%d REV2=%d), defaulting to 4 GB\n",
		       rev0, rev1, rev2);
	return 4;
}

int board_late_init(void)
{
	ti_set_fdt_env(NULL, NULL);

	/* Override fdtfile based on LPDDR4 size encoded in HW_REV GPIO straps */
	switch (mo62a_get_lpddr4_gb()) {
	case 2:
		env_set("fdtfile", "ti/k3-am62a7-mo-62a-2gb.dtb");
		break;
	case 4:
		env_set("fdtfile", "ti/k3-am62a7-mo-62a-4gb.dtb");
		break;
	case 8:
		env_set("fdtfile", "ti/k3-am62a7-mo-62a-8gb.dtb");
		break;
	default: /* fallback: keep whatever ti_set_fdt_env set */
		break;
	}
	printf("MO-62A: fdtfile=%s\n", env_get("fdtfile"));

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
