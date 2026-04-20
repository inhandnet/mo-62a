// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM62PX Processor Info
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_processors_info am62px_processors_info[] = {
	{121, 0, 0x01, "WKUP_R5FSS0_CORE0"},
	{9,   0, 0x03, "MCU_R5FSS0_CORE0"},
	{135, 0, 0x20, "A53SS0_CORE_0"},
	{136, 0, 0x21, "A53SS0_CORE_1"},
	{137, 0, 0x22, "A53SS0_CORE_2"},
	{138, 0, 0x23, "A53SS0_CORE_3"},
	{225, 0, 0x80, "HSM0"},
};
