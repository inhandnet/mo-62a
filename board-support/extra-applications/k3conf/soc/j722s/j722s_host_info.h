/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * J722S Host Info
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __J722S_HOST_INFO_H
#define __J722S_HOST_INFO_H

#define J722S_HOST_ID_TIFS	0
#define J722S_HOST_ID_A53_0	10
#define J722S_HOST_ID_A53_1	11
#define J722S_HOST_ID_A53_2	12
#define J722S_HOST_ID_A53_3	13
#define J722S_HOST_ID_A53_4	14
#define J722S_HOST_ID_C7X_0_0	20
#define J722S_HOST_ID_C7X_1_0	22
#define J722S_HOST_ID_MCU_0_R5_0	30
#define J722S_HOST_ID_GPU_0	31
#define J722S_HOST_ID_GPU_1	32
#define J722S_HOST_ID_WKUP_0_R5_0	35
#define J722S_HOST_ID_WKUP_0_R5_1	36
#define J722S_HOST_ID_MAIN_0_R5_0	37
#define J722S_HOST_ID_MAIN_0_R5_1	38
#define J722S_HOST_ID_DM2TIFS	250
#define J722S_HOST_ID_TIFS2DM	251
#define J722S_HOST_ID_HSM	253
#define J722S_HOST_ID_DM	254

#define J722S_MAX_HOST_IDS	19

extern struct ti_sci_host_info j722s_host_info[];

#endif /* __J722S_HOST_INFO_H */
