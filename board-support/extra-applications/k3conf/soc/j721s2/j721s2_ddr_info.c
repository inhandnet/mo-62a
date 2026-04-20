// SPDX-License-Identifier: BSD-3-Clause
/*
 * J721S2 DDR performance information
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <ddr_perf.h>

static uintptr_t j721s2_ddr_base_address[] = {
	0x02980100,
	0x029A0100,
};

struct ddr_perf_soc_info j721s2_ddr_perf_info = {
	.num_perf_insts = ARRAY_SIZE(j721s2_ddr_base_address),
	.burst_size = 64,
	.perf_inst_base = j721s2_ddr_base_address,
};
