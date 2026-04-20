/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * J7200 Host Info
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __J7200_HOST_INFO_H
#define __J7200_HOST_INFO_H

#define J7200_HOST_ID_DMSC	0
#define J7200_HOST_ID_MCU_0_R5_0	3
#define J7200_HOST_ID_MCU_0_R5_1	4
#define J7200_HOST_ID_MCU_0_R5_2	5
#define J7200_HOST_ID_MCU_0_R5_3	6
#define J7200_HOST_ID_A72_0	10
#define J7200_HOST_ID_A72_1	11
#define J7200_HOST_ID_A72_2	12
#define J7200_HOST_ID_A72_3	13
#define J7200_HOST_ID_A72_4	14
#define J7200_HOST_ID_MAIN_0_R5_0	35
#define J7200_HOST_ID_MAIN_0_R5_1	36
#define J7200_HOST_ID_MAIN_0_R5_2	37
#define J7200_HOST_ID_MAIN_0_R5_3	38
#define J7200_HOST_ID_DM2DMSC	250
#define J7200_HOST_ID_DMSC2DM	251
#define J7200_HOST_ID_DM	254

#define J7200_MAX_HOST_IDS	17

extern struct ti_sci_host_info j7200_host_info[];

#endif /* __J7200_HOST_INFO_H */
