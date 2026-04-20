// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM62X Hosts Info
 *
 * Copyright (C) 2022-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_host_info am62x_host_info[] = {
	{0, "TIFS", "Secure", "Device Management and Security Control"},
	{10, "A53_0", "Secure", "Cortex a53 context 0 on Main island"},
	{11, "A53_1", "Secure", "Cortex A53 context 1 on Main island"},
	{12, "A53_2", "Non Secure", "Cortex A53 context 2 on Main island"},
	{13, "A53_3", "Non Secure", "Cortex A53 context 3 on Main island"},
	{14, "A53_4", "Non Secure", "Cortex A53 context 1 on Main island"},
	{30, "M4_0", "Non Secure", "M4"},
	{31, "GPU", "Non Secure", "GPU context 0 on Main island"},
	{35, "MAIN_0_R5_0", "Secure", "Cortex R5_0 context 0 on Main island(BOOT)"},
	{36, "MAIN_0_R5_1", "Non Secure", "Cortex R5_0 context 1 on Main island"},
	{37, "MAIN_0_R5_2", "Secure", "Cortex R5_0 context 2 on Main island"},
	{38, "MAIN_0_R5_3", "Non Secure", "Cortex R5_0 context 3 on Main island"},
	{250, "DM2TIFS", "Secure", "DM to TIFS communication"},
	{251, "TIFS2DM", "Non Secure", "TIFS to DM communication"},
	{253, "HSM", "Secure", "HSM Controller"},
	{254, "DM", "Non Secure", "Device Management"},
};
