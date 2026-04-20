/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * J721S2 Sec Proxy Info
 *
 * Copyright (C) 2022-2025 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __J721S2_SEC_PROXY_INFO_H
#define __J721S2_SEC_PROXY_INFO_H

#define J721S2_MAIN_SEC_PROXY_THREADS	113
#define J721S2_MCU_SEC_PROXY_THREADS	39

extern struct ti_sci_sec_proxy_info j721s2_main_sp_info[];
extern struct ti_sci_sec_proxy_info j721s2_mcu_sp_info[];

#endif /* __J721S2_SEC_PROXY_INFO_H */
