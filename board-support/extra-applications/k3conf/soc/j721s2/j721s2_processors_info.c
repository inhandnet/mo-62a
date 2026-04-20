// SPDX-License-Identifier: BSD-3-Clause
/*
 * J721S2 Processor Info
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_processors_info j721s2_processors_info[] = {
	{202, 0, 0x20, "A72SS0_CORE0"},
	{203, 0, 0x21, "A72SS0_CORE1"},
	{8, 0, 0x30, "COMPUTE_CLUSTER0_C71SS0_0"},
	{11, 0, 0x31, "COMPUTE_CLUSTER0_C71SS1_0"},
	{284, 0, 0x01, "MCU_R5FSS0_CORE0"},
	{285, 0, 0x02, "MCU_R5FSS0_CORE1"},
	{279, 0, 0x06, "R5FSS0_CORE0"},
	{280, 0, 0x07, "R5FSS0_CORE1"},
	{281, 0, 0x08, "R5FSS1_CORE0"},
	{282, 0, 0x09, "R5FSS1_CORE1"},
	{304, 0, 0x80, "WKUP_HSM0"},
};
