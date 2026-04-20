// SPDX-License-Identifier: BSD-3-Clause
/*
 * AM62PX DDR performance information
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <ddr_perf.h>

static uintptr_t am62p_ddr_base_address[] = {
	0x0F300100,
};

struct ddr_perf_soc_info am62px_ddr_perf_info = {
	.num_perf_insts = ARRAY_SIZE(am62p_ddr_base_address),
	.burst_size = 32,
	.perf_inst_base = am62p_ddr_base_address,
};
