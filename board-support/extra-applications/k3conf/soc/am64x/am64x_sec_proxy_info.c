// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM64X Sec Proxy Info
 *
 * Copyright (C) 2020-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_sec_proxy_info am64x_main_sp_info[] = {
	{0, "read", 11, "MAIN_0_R5_0", "response"},
	{1, "write", 10, "MAIN_0_R5_0", "low_priority"},
	{2, "read", 11, "MAIN_0_R5_1", "response"},
	{3, "write", 10, "MAIN_0_R5_1", "low_priority"},
	{4, "read", 2, "MAIN_0_R5_2", "response"},
	{5, "write", 1, "MAIN_0_R5_2", "low_priority"},
	{6, "read", 2, "MAIN_0_R5_3", "response"},
	{7, "write", 1, "MAIN_0_R5_3", "low_priority"},
	{8, "read", 11, "A53_0", "response"},
	{9, "write", 10, "A53_0", "low_priority"},
	{10, "read", 11, "A53_1", "response"},
	{11, "write", 10, "A53_1", "low_priority"},
	{12, "read", 6, "A53_2", "response"},
	{13, "write", 5, "A53_2", "low_priority"},
	{14, "read", 6, "A53_3", "response"},
	{15, "write", 5, "A53_3", "low_priority"},
	{16, "read", 6, "M4_0", "response"},
	{17, "write", 5, "M4_0", "low_priority"},
	{18, "read", 6, "MAIN_1_R5_0", "response"},
	{19, "write", 5, "MAIN_1_R5_0", "low_priority"},
	{20, "read", 6, "MAIN_1_R5_1", "response"},
	{21, "write", 5, "MAIN_1_R5_1", "low_priority"},
	{22, "read", 2, "MAIN_1_R5_2", "response"},
	{23, "write", 1, "MAIN_1_R5_2", "low_priority"},
	{24, "read", 2, "MAIN_1_R5_3", "response"},
	{25, "write", 1, "MAIN_1_R5_3", "low_priority"},
	{26, "read", 6, "A53_4", "response"},
	{27, "write", 5, "A53_4", "low_priority"},
	{28, "read", 2, "ICSSG_0", "response"},
	{29, "write", 1, "ICSSG_0", "low_priority"},
	{30, "read", 2, "ICSSG_1", "response"},
	{31, "write", 1, "ICSSG_1", "low_priority"},
};
