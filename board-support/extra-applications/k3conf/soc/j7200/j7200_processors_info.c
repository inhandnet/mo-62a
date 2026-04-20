// SPDX-License-Identifier: BSD-3-Clause
/*
 * J7200 Processor Info
 *
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_processors_info j7200_processors_info[] = {
	{202, 2, 0x20, "A72SS0_CORE0_0"},
	{203, 0, 0x21, "A72SS0_CORE0_1"},
	{250, 0, 0x01, "MCU_R5FSS0_CORE0"},
	{251, 0, 0x02, "MCU_R5FSS0_CORE1"},
	{245, 0, 0x06, "R5FSS0_CORE0"},
	{246, 0, 0x07, "R5FSS0_CORE1"},
};
