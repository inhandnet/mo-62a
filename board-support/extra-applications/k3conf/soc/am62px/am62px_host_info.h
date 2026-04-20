/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * AM62PX Host Info
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __AM62PX_HOST_INFO_H
#define __AM62PX_HOST_INFO_H

#define AM62PX_HOST_ID_TIFS	0
#define AM62PX_HOST_ID_A53_0	10
#define AM62PX_HOST_ID_A53_1	11
#define AM62PX_HOST_ID_A53_2	12
#define AM62PX_HOST_ID_A53_3	13
#define AM62PX_HOST_ID_A53_4	14
#define AM62PX_HOST_ID_MCU_0_R5_0	30
#define AM62PX_HOST_ID_GPU_0	31
#define AM62PX_HOST_ID_GPU_1	32
#define AM62PX_HOST_ID_WKUP_0_R5_0	35
#define AM62PX_HOST_ID_WKUP_0_R5_1	36
#define AM62PX_HOST_ID_DM2TIFS	250
#define AM62PX_HOST_ID_TIFS2DM	251
#define AM62PX_HOST_ID_HSM	253
#define AM62PX_HOST_ID_DM	254

#define AM62PX_MAX_HOST_IDS	15

extern struct ti_sci_host_info am62px_host_info[];

#endif /* __AM62PX_HOST_INFO_H */
