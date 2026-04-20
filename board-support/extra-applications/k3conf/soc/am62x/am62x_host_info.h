/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * AM62X Host Info
 *
 * Copyright (C) 2022-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __AM62X_HOST_INFO_H
#define __AM62X_HOST_INFO_H

#define AM62X_HOST_ID_TIFS	0
#define AM62X_HOST_ID_A53_0	10
#define AM62X_HOST_ID_A53_1	11
#define AM62X_HOST_ID_A53_2	12
#define AM62X_HOST_ID_A53_3	13
#define AM62X_HOST_ID_A53_4	14
#define AM62X_HOST_ID_M4_0	30
#define AM62X_HOST_ID_GPU	31
#define AM62X_HOST_ID_MAIN_0_R5_0	35
#define AM62X_HOST_ID_MAIN_0_R5_1	36
#define AM62X_HOST_ID_MAIN_0_R5_2	37
#define AM62X_HOST_ID_MAIN_0_R5_3	38
#define AM62X_HOST_ID_DM2TIFS	250
#define AM62X_HOST_ID_TIFS2DM	251
#define AM62X_HOST_ID_HSM	253
#define AM62X_HOST_ID_DM	254

#define AM62X_MAX_HOST_IDS	16

extern struct ti_sci_host_info am62x_host_info[];

#endif /* __AM62X_HOST_INFO_H */
