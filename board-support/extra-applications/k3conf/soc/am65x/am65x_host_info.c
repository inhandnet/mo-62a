// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM65X Hosts Info
 *
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_host_info am65x_host_info[] = {
	{0, "DMSC", "Secure", "Device Management and Security Control"},
	{3, "R5_0", "Non Secure", "Cortex R5 Context 0 on MCU island"},
	{4, "R5_1", "Secure", "Cortex R5 Context 1 on MCU island(Boot)"},
	{5, "R5_2", "Non Secure", "Cortex R5 Context 2 on MCU island"},
	{6, "R5_3", "Secure", "Cortex R5 Context 3 on MCU island"},
	{10, "A53_0", "Secure", "Cortex A53 context 0 on Main island"},
	{11, "A53_1", "Secure", "Cortex A53 context 1 on Main island"},
	{12, "A53_2", "Non Secure", "Cortex A53 context 2 on Main island"},
	{13, "A53_3", "Non Secure", "Cortex A53 context 3 on Main island"},
	{14, "A53_4", "Non Secure", "Cortex A53 context 4 on Main island"},
	{15, "A53_5", "Non Secure", "Cortex A53 context 5 on Main island"},
	{16, "A53_6", "Non Secure", "Cortex A53 context 6 on Main island"},
	{17, "A53_7", "Non Secure", "Cortex A53 context 7 on Main island"},
	{30, "GPU_0", "Non Secure", "SGX544 Context 0 on Main island"},
	{31, "GPU_1", "Non Secure", "SGX544 Context 1 on Main island"},
	{50, "ICSSG_0", "Non Secure", "ICSS Context 0 on Main island"},
	{51, "ICSSG_1", "Non Secure", "ICSS Context 1 on Main island"},
	{52, "ICSSG_2", "Non Secure", "ICSS Context 2 on Main island"},
};
