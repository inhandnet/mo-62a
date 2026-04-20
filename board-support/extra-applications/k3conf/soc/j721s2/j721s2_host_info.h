/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * J721S2 Host Info
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __J721S2_HOST_INFO_H
#define __J721S2_HOST_INFO_H

#define J721S2_HOST_ID_TIFS	0
#define J721S2_HOST_ID_MCU_0_R5_0	3
#define J721S2_HOST_ID_MCU_0_R5_1	4
#define J721S2_HOST_ID_MCU_0_R5_2	5
#define J721S2_HOST_ID_MCU_0_R5_3	6
#define J721S2_HOST_ID_A72_0	10
#define J721S2_HOST_ID_A72_1	11
#define J721S2_HOST_ID_A72_2	12
#define J721S2_HOST_ID_A72_3	13
#define J721S2_HOST_ID_A72_4	14
#define J721S2_HOST_ID_C7X_0_0	20
#define J721S2_HOST_ID_C7X_0_1	21
#define J721S2_HOST_ID_C7X_1_0	22
#define J721S2_HOST_ID_C7X_1_1	23
#define J721S2_HOST_ID_GPU_0	30
#define J721S2_HOST_ID_MAIN_0_R5_0	35
#define J721S2_HOST_ID_MAIN_0_R5_1	36
#define J721S2_HOST_ID_MAIN_0_R5_2	37
#define J721S2_HOST_ID_MAIN_0_R5_3	38
#define J721S2_HOST_ID_MAIN_1_R5_0	40
#define J721S2_HOST_ID_MAIN_1_R5_1	41
#define J721S2_HOST_ID_MAIN_1_R5_2	42
#define J721S2_HOST_ID_MAIN_1_R5_3	43
#define J721S2_HOST_ID_DM2TIFS	250
#define J721S2_HOST_ID_TIFS2DM	251
#define J721S2_HOST_ID_HSM	253
#define J721S2_HOST_ID_DM	254

#define J721S2_MAX_HOST_IDS	27

extern struct ti_sci_host_info j721s2_host_info[];

#endif /* __J721S2_HOST_INFO_H */
