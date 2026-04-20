// SPDX-License-Identifier: BSD-3-Clause
/*
 * J784S4 Processor Info
 *
 * Copyright (C) 2022-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_processors_info j784s4_processors_info[] = {
	{202, 0, 0x20, "A72SS0_CORE0"},
	{203, 0, 0x21, "A72SS0_CORE1"},
	{204, 0, 0x22, "A72SS0_CORE2"},
	{205, 0, 0x23, "A72SS0_CORE3"},
	{206, 0, 0x24, "A72SS1_CORE0"},
	{207, 0, 0x25, "A72SS1_CORE1"},
	{208, 0, 0x26, "A72SS1_CORE2"},
	{209, 0, 0x27, "A72SS1_CORE3"},
	{31, 0, 0x30, "COMPUTE_CLUSTER0_C71SS0_CORE0"},
	{34, 0, 0x31, "COMPUTE_CLUSTER0_C71SS1_CORE0"},
	{38, 0, 0x32, "COMPUTE_CLUSTER0_C71SS2_CORE0"},
	{41, 0, 0x33, "COMPUTE_CLUSTER0_C71SS3_CORE0"},
	{346, 0, 0x01, "MCU_R5FSS0_CORE0"},
	{347, 0, 0x02, "MCU_R5FSS0_CORE1"},
	{339, 0, 0x06, "R5FSS0_CORE0"},
	{340, 0, 0x07, "R5FSS0_CORE1"},
	{341, 0, 0x08, "R5FSS1_CORE0"},
	{342, 0, 0x09, "R5FSS1_CORE1"},
	{343, 0, 0x0A, "R5FSS2_CORE0"},
	{344, 0, 0x0B, "R5FSS2_CORE1"},
	{371, 0, 0x80, "WKUP_HSM0"},
};
