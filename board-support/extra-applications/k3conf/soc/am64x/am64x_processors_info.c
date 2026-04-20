// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM64X Processor Info
 *
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_processors_info am64x_processors_info[] = {
	{135, 0, 0x20, "A53SS0_CORE_0"},
	{136, 0, 0x21, "A53SS0_CORE_1"},
	{9, 0, 0x18, "MCU_M4FSS0_CORE0"},
	{121, 0, 0x01, "R5FSS0_CORE0"},
	{122, 0, 0x02, "R5FSS0_CORE1"},
	{123, 0, 0x06, "R5FSS1_CORE0"},
	{124, 0, 0x07, "R5FSS1_CORE1"},
};
