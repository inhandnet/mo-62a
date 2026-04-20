/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * AM62LX DDR performance information
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __AM62LX_DDRBW_INFO_H
#define __AM62LX_DDRBW_INFO_H

#define AM62LX_DDR_CLASS_REG 0x0F308000
#define AM62LX_DDR_CLASS_MASK(val) ((val & 0x00000F00) >> 8)
#define AM62LX_DDR_TYPE_DDR4 0xA
#define AM62LX_DDR_TYPE_LPDDR4 0xB

extern struct ddr_perf_soc_info am62lx_ddr_perf_info;

#endif /* __AM62LX_DDRBW_INFO_H */
