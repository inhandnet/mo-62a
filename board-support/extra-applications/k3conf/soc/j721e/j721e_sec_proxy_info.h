/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * J721E Sec Proxy Info
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __J721E_SEC_PROXY_INFO_H
#define __J721E_SEC_PROXY_INFO_H

#define J721E_MAIN_SEC_PROXY_THREADS	132
#define J721E_MCU_SEC_PROXY_THREADS	39

extern struct ti_sci_sec_proxy_info j721e_main_sp_info[];
extern struct ti_sci_sec_proxy_info j721e_mcu_sp_info[];

#endif /* __J721E_SEC_PROXY_INFO_H */
