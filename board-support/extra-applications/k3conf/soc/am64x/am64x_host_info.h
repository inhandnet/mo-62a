/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * AM64X Host Info
 *
 * Copyright (C) 2020-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __AM64X_HOST_INFO_H
#define __AM64X_HOST_INFO_H

#define AM64X_HOST_ID_DMSC	0
#define AM64X_HOST_ID_A53_0	10
#define AM64X_HOST_ID_A53_1	11
#define AM64X_HOST_ID_A53_2	12
#define AM64X_HOST_ID_A53_3	13
#define AM64X_HOST_ID_A53_4	14
#define AM64X_HOST_ID_M4_0	30
#define AM64X_HOST_ID_MAIN_0_R5_0	35
#define AM64X_HOST_ID_MAIN_0_R5_1	36
#define AM64X_HOST_ID_MAIN_0_R5_2	37
#define AM64X_HOST_ID_MAIN_0_R5_3	38
#define AM64X_HOST_ID_MAIN_1_R5_0	40
#define AM64X_HOST_ID_MAIN_1_R5_1	41
#define AM64X_HOST_ID_MAIN_1_R5_2	42
#define AM64X_HOST_ID_MAIN_1_R5_3	43
#define AM64X_HOST_ID_ICSSG_0	50
#define AM64X_HOST_ID_ICSSG_1	51

#define AM64X_MAX_HOST_IDS	17

extern struct ti_sci_host_info am64x_host_info[];

#endif /* __AM64X_HOST_INFO_H */
