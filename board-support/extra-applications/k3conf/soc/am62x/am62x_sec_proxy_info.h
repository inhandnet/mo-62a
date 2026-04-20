/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * AM62X Sec Proxy Info
 *
 * Copyright (C) 2022-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __AM62X_SEC_PROXY_INFO_H
#define __AM62X_SEC_PROXY_INFO_H

#define AM62X_MAIN_SEC_PROXY_THREADS	35
#define AM62X_MCU_SEC_PROXY_THREADS	4

extern struct ti_sci_sec_proxy_info am62x_main_sp_info[];
extern struct ti_sci_sec_proxy_info am62x_mcu_sp_info[];

#endif /* __AM62X_SEC_PROXY_INFO_H */
