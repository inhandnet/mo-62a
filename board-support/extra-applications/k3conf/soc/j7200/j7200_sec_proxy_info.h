/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * J7200 Sec Proxy Info
 *
 * Copyright (C) 2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __J7200_SEC_PROXY_INFO_H
#define __J7200_SEC_PROXY_INFO_H

#define J7200_MAIN_SEC_PROXY_THREADS	58
#define J7200_MCU_SEC_PROXY_THREADS	39

extern struct ti_sci_sec_proxy_info j7200_main_sp_info[];
extern struct ti_sci_sec_proxy_info j7200_mcu_sp_info[];

#endif /* __J7200_SEC_PROXY_INFO_H */
