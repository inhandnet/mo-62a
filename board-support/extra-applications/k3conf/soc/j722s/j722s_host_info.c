// SPDX-License-Identifier: BSD-3-Clause
/*
 * J722S Hosts Info
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_host_info j722s_host_info[] = {
	{0, "TIFS", "Secure", "TI Foundational Security"},
	{10, "A53_0", "Secure", "Cortex A53 context 0 on MAIN domain"},
	{11, "A53_1", "Secure", "Cortex A53 context 1 on MAIN domain"},
	{12, "A53_2", "Non Secure", "Cortex A53 context 2 on MAIN domain"},
	{13, "A53_3", "Non Secure", "Cortex A53 context 3 on MAIN domain"},
	{14, "A53_4", "Non Secure", "Cortex A53 context 4 on MAIN domain"},
	{20, "C7X_0_0", "Non Secure", "C7x_0 context 0 on MAIN domain"},
	{22, "C7X_1_0", "Non Secure", "C7x_1 context 0 on MAIN domain"},
	{30, "MCU_0_R5_0", "Non Secure", "MCU R5"},
	{31, "GPU_0", "Non Secure", "GPU context 0 on MAIN domain"},
	{32, "GPU_1", "Non Secure", "GPU context 1 on MAIN domain"},
	{35, "WKUP_0_R5_0", "Secure", "Cortex R5_0 context 0 on WKUP domain (BOOT)"},
	{36, "WKUP_0_R5_1", "Non Secure", "Cortex R5_0 context 1 on WKUP domain"},
	{37, "MAIN_0_R5_0", "Secure", "Cortex R5_0 context 0 on MAIN domain"},
	{38, "MAIN_0_R5_1", "Non Secure", "Cortex R5_0 context 1 on MAIN domain"},
	{250, "DM2TIFS", "Secure", "DM to TIFS communication"},
	{251, "TIFS2DM", "Non Secure", "TIFS to DM communication"},
	{253, "HSM", "Secure", "HSM Controller"},
	{254, "DM", "Non Secure", "Device Management"},
};
