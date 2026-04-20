/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * J722S Sec Proxy Info
 *
 * Copyright (C) 2024 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __J722S_SEC_PROXY_INFO_H
#define __J722S_SEC_PROXY_INFO_H

#define J722S_MAIN_SEC_PROXY_THREADS	44
#define J722S_MCU_SEC_PROXY_THREADS	4

extern struct ti_sci_sec_proxy_info j722s_main_sp_info[];
extern struct ti_sci_sec_proxy_info j722s_mcu_sp_info[];

#endif /* __J722S_SEC_PROXY_INFO_H */
