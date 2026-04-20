// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM65X Processor Info
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <tisci.h>
#include <socinfo.h>

struct ti_sci_processors_info am65x_processors_info[] = {
	{202, 0, 0x20, "A53_CL0_C0"},
	{203, 0, 0x21, "A53_CL0_C1"},
	{204, 0, 0x22, "A53_CL1_C0"},
	{205, 0, 0x23, "A53_CL1_C1"},
	{159, 0, 0x01, "R5_CL0_C0"},
	{245, 0, 0x02, "R5_CL0_C1"},
};
