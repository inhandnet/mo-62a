// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM62X Processor Info
 *
 * Copyright (C) 2022-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_processors_info am62x_processors_info[] = {
	{135, 0, 0x20, "A53SS0_CORE_0"},
	{136, 0, 0x21, "A53SS0_CORE_1"},
	{137, 0, 0x22, "A53SS0_CORE_2"},
	{138, 0, 0x23, "A53SS0_CORE_3"},
	{9  , 1, 0x18, "MCU_M4FSS0_CORE0"},
	{121, 1, 0x01, "R5FSS0_CORE0"},
	{225, 0, 0x80, "HSM0"},
};
