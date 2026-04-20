// SPDX-License-Identifier: BSD-3-Clause
/*
 * J7200 DDR performance information
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <ddr_perf.h>

static uintptr_t j7200_ddr_base_address[] = {
	0x02980100,
};

struct ddr_perf_soc_info j7200_ddr_perf_info = {
	.num_perf_insts = ARRAY_SIZE(j7200_ddr_base_address),
	.burst_size = 64,
	.perf_inst_base = j7200_ddr_base_address,
};
