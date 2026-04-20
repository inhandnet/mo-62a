/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * AM65X Host Info
 *
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __AM65X_HOST_INFO_H
#define __AM65X_HOST_INFO_H

#define AM65X_HOST_ID_DMSC	0
#define AM65X_HOST_ID_R5_0	3
#define AM65X_HOST_ID_R5_1	4
#define AM65X_HOST_ID_R5_2	5
#define AM65X_HOST_ID_R5_3	6
#define AM65X_HOST_ID_A53_0	10
#define AM65X_HOST_ID_A53_1	11
#define AM65X_HOST_ID_A53_2	12
#define AM65X_HOST_ID_A53_3	13
#define AM65X_HOST_ID_A53_4	14
#define AM65X_HOST_ID_A53_5	15
#define AM65X_HOST_ID_A53_6	16
#define AM65X_HOST_ID_A53_7	17
#define AM65X_HOST_ID_GPU_0	30
#define AM65X_HOST_ID_GPU_1	31
#define AM65X_HOST_ID_ICSSG_0	50
#define AM65X_HOST_ID_ICSSG_1	51
#define AM65X_HOST_ID_ICSSG_2	52

#define AM65X_MAX_HOST_IDS	18

extern struct ti_sci_host_info am65x_host_info[];

#endif /* __AM65X_HOST_INFO_H */
