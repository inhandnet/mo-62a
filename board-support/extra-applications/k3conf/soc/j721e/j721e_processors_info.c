// SPDX-License-Identifier: BSD-3-Clause
/*
 * J721E Processor Info
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_processors_info j721e_processors_info[] = {
	{202, 2, 0x20, "A72SS0_CORE0"},
	{203, 0, 0x21, "A72SS0_CORE1"},
	{142, 6, 0x03, "C66SS0_CORE0"},
	{143, 6, 0x04, "C66SS1_CORE0"},
	{15, 0, 0x30, "C71SS0"},
	{250, 0, 0x01, "MCU_R5FSS0_CORE0"},
	{251, 0, 0x02, "MCU_R5FSS0_CORE1"},
	{245, 0, 0x06, "R5FSS0_CORE0"},
	{246, 0, 0x07, "R5FSS0_CORE1"},
	{247, 0, 0x08, "R5FSS1_CORE0"},
	{248, 0, 0x09, "R5FSS1_CORE1"},
};
