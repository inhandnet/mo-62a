// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM64X Hosts Info
 *
 * Copyright (C) 2020-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_host_info am64x_host_info[] = {
	{0, "DMSC", "Secure", "Device Management and Security Control"},
	{10, "A53_0", "Secure", "Cortex a53 context 0 on Main island"},
	{11, "A53_1", "Secure", "Cortex A53 context 1 on Main island"},
	{12, "A53_2", "Non Secure", "Cortex A53 context 2 on Main island"},
	{13, "A53_3", "Non Secure", "Cortex A53 context 3 on Main island"},
	{14, "A53_4", "Non Secure", "Cortex A53 context 1 on Main island"},
	{30, "M4_0", "Non Secure", "M4"},
	{35, "MAIN_0_R5_0", "Secure", "Cortex R5_0 context 0 on Main island(BOOT)"},
	{36, "MAIN_0_R5_1", "Non Secure", "Cortex R5_0 context 1 on Main island"},
	{37, "MAIN_0_R5_2", "Secure", "Cortex R5_0 context 2 on Main island"},
	{38, "MAIN_0_R5_3", "Non Secure", "Cortex R5_0 context 3 on Main island"},
	{40, "MAIN_1_R5_0", "Secure", "Cortex R5_1 context 0 on Main island"},
	{41, "MAIN_1_R5_1", "Non Secure", "Cortex R5_1 context 1 on Main island"},
	{42, "MAIN_1_R5_2", "Secure", "Cortex R5_1 context 2 on Main island"},
	{43, "MAIN_1_R5_3", "Non Secure", "Cortex R5_1 context 3 on Main island"},
	{50, "ICSSG_0", "Non Secure", "ICSSG context 0 on Main island"},
	{51, "ICSSG_1", "Non Secure", "ICSSG context 1 on Main island"},
};
