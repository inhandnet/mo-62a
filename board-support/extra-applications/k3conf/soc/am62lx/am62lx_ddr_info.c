// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM62LX DDR performance information
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <ddr_perf.h>

static uintptr_t am62l_ddr_base_address[] = {
	0x0F300100,
};

struct ddr_perf_soc_info am62lx_ddr_perf_info = {
	.num_perf_insts = ARRAY_SIZE(am62l_ddr_base_address),
	.burst_size = 16,
	.perf_inst_base = am62l_ddr_base_address,
};
