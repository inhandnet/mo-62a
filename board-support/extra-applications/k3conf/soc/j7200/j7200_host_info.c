// SPDX-License-Identifier: BSD-3-Clause
/*
 * J7200 Hosts Info
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_host_info j7200_host_info[] = {
	{0, "DMSC", "Secure", "Security Controller"},
	{3, "MCU_0_R5_0", "Non Secure", "Cortex R5 context 0 on MCU island"},
	{4, "MCU_0_R5_1", "Secure", "Cortex R5 context 1 on MCU island(Boot)"},
	{5, "MCU_0_R5_2", "Non Secure", "Cortex R5 context 2 on MCU island"},
	{6, "MCU_0_R5_3", "Secure", "Cortex R5 context 3 on MCU island"},
	{10, "A72_0", "Secure", "Cortex A72 context 0 on Main island"},
	{11, "A72_1", "Secure", "Cortex A72 context 1 on Main island"},
	{12, "A72_2", "Non Secure", "Cortex A72 context 2 on Main island"},
	{13, "A72_3", "Non Secure", "Cortex A72 context 3 on Main island"},
	{14, "A72_4", "Non Secure", "Cortex A72 context 4 on Main island"},
	{35, "MAIN_0_R5_0", "Non Secure", "Cortex R5_0 context 0 on Main island"},
	{36, "MAIN_0_R5_1", "Secure", "Cortex R5_0 context 1 on Main island"},
	{37, "MAIN_0_R5_2", "Non Secure", "Cortex R5_0 context 2 on Main island"},
	{38, "MAIN_0_R5_3", "Secure", "Cortex R5_0 context 3 on Main island"},
	{250, "DM2DMSC", "Secure", "DM to DMSC communication"},
	{251, "DMSC2DM", "Non Secure", "DMSC to DM communication"},
	{254, "DM", "Non Secure", "Device Management"},
};
