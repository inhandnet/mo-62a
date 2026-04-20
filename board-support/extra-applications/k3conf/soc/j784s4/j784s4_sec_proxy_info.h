/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * J784S4 Sec Proxy Info
 *
 * Copyright (C) 2022-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __J784S4_SEC_PROXY_INFO_H
#define __J784S4_SEC_PROXY_INFO_H

#define J784S4_MAIN_SEC_PROXY_THREADS	182
#define J784S4_MCU_SEC_PROXY_THREADS	39

extern struct ti_sci_sec_proxy_info j784s4_main_sp_info[];
extern struct ti_sci_sec_proxy_info j784s4_mcu_sp_info[];

#endif /* __J784S4_SEC_PROXY_INFO_H */
