/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * AM62PX Sec Proxy Info
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __AM62PX_SEC_PROXY_INFO_H
#define __AM62PX_SEC_PROXY_INFO_H

#define AM62PX_MAIN_SEC_PROXY_THREADS	33
#define AM62PX_MCU_SEC_PROXY_THREADS	4

extern struct ti_sci_sec_proxy_info am62px_main_sp_info[];
extern struct ti_sci_sec_proxy_info am62px_mcu_sp_info[];

#endif /* __AM62PX_SEC_PROXY_INFO_H */
