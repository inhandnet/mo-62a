/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * AM65X_SR2 Sec Proxy Info
 *
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef __AM65X_SR2_SEC_PROXY_INFO_H
#define __AM65X_SR2_SEC_PROXY_INFO_H

#define AM65X_SR2_MAIN_SEC_PROXY_THREADS	65
#define AM65X_SR2_MCU_SEC_PROXY_THREADS	20

extern struct ti_sci_sec_proxy_info am65x_sr2_main_sp_info[];
extern struct ti_sci_sec_proxy_info am65x_sr2_mcu_sp_info[];

#endif /* __AM65X_SR2_SEC_PROXY_INFO_H */
