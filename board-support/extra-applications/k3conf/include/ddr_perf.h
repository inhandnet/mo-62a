/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * DDR performance counter information
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 *	Aarya Chaumal <a-chaumal@ti.com>
 */

#ifndef __DDRPERF_H
#define __DDRPERF_H

#include <stdint.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

struct ddr_perf_soc_info {
	uint8_t num_perf_insts;
	uint8_t burst_size;
	uintptr_t *perf_inst_base;
};

#endif
