// SPDX-License-Identifier: BSD-3-Clause
/*
 * J784S4 DDR performance information
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <ddr_perf.h>

static uintptr_t j784s4_ddr_base_address[] = {
	0x02980100,
	0x029A0100,
	0x029C0100,
	0x029E0100,
};

struct ddr_perf_soc_info j784s4_ddr_perf_info = {
	.num_perf_insts = ARRAY_SIZE(j784s4_ddr_base_address),
	.burst_size = 64,
	.perf_inst_base = j784s4_ddr_base_address,
};

static uintptr_t j742s2_ddr_base_address[] = {
	0x02980100,
	0x029A0100,
};

struct ddr_perf_soc_info j742s2_ddr_perf_info = {
	.num_perf_insts = ARRAY_SIZE(j742s2_ddr_base_address),
	.burst_size = 64,
	.perf_inst_base = j742s2_ddr_base_address,
};
