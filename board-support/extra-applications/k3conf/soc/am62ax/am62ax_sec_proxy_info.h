/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * AM62AX Sec Proxy Info
 *
 * Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __AM62AX_SEC_PROXY_INFO_H
#define __AM62AX_SEC_PROXY_INFO_H

#define AM62AX_MAIN_SEC_PROXY_THREADS	35
#define AM62AX_MCU_SEC_PROXY_THREADS	4

extern struct ti_sci_sec_proxy_info am62ax_main_sp_info[];
extern struct ti_sci_sec_proxy_info am62ax_mcu_sp_info[];

#endif /* __AM62AX_SEC_PROXY_INFO_H */
