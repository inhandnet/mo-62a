/*
 *
 * Copyright (c) 2018 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef APP_SCICLIENT_WRAPPER_API_H_
#define APP_SCICLIENT_WRAPPER_API_H_

#if !defined(MCU_PLUS_SDK)
#include <sciclient/sciclient.h>
#else
#include <SystemP.h>
#define SCICLIENT_SERVICE_WAIT_FOREVER SystemP_WAIT_FOREVER
#endif

#include <utils/console_io/include/app_log.h>

/* make below 0 to disable debug print's for these macro APIs */
#define APP_DEBUG_SCICLIENT 1

#define SET_CLOCK_PARENT(MOD, CLK, PARENT) do { \
    int32_t status = 0; \
    if(status == 0) { \
        if(APP_DEBUG_SCICLIENT) \
            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkParent module=%u clk=%u parent=%u\n", MOD, CLK, PARENT); \
        status = Sciclient_pmSetModuleClkParent(MOD, CLK, PARENT, SCICLIENT_SERVICE_WAIT_FOREVER); \
        if(status != 0) appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleClkParent failed\n"); \
        else \
        { \
            if(APP_DEBUG_SCICLIENT) \
                appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkParent success\n"); \
        } \
    } \
} while(0)

#define SET_DEVICE_STATE(MOD, STATE) do { \
    int32_t status = 0; \
    if(status == 0) { \
        if(APP_DEBUG_SCICLIENT) \
            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState module=%u state=%u\n", MOD, STATE); \
        status = Sciclient_pmSetModuleState(MOD, STATE, TISCI_MSG_FLAG_AOP, SCICLIENT_SERVICE_WAIT_FOREVER); \
        if(status != 0) appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleState failed\n"); \
        else \
        { \
            if(APP_DEBUG_SCICLIENT) \
                appLogPrintf("SCICLIENT: Sciclient_pmSetModuleState success\n"); \
        } \
    } \
} while(0)

#define SET_DEVICE_STATE_ON(MOD) SET_DEVICE_STATE(MOD,TISCI_MSG_VALUE_DEVICE_SW_STATE_ON)

#define SET_DEVICE_STATE_OFF(MOD) SET_DEVICE_STATE(MOD,TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF)

#define SET_CLOCK_STATE(MOD, CLK, FLAG, STATE) do { \
    int32_t status = 0; \
    if(status == 0) { \
        if(APP_DEBUG_SCICLIENT) \
            appLogPrintf("SCICLIENT: Sciclient_pmModuleClkRequest module=%u clk=%u state=%u flag=%u\n", MOD, CLK, STATE, FLAG); \
        status = Sciclient_pmModuleClkRequest(MOD, CLK, STATE, FLAG, SCICLIENT_SERVICE_WAIT_FOREVER); \
        if(status != 0) appLogPrintf("SCICLIENT: ERROR: Sciclient_pmModuleClkRequest failed\n"); \
        else \
        { \
            if(APP_DEBUG_SCICLIENT) \
                appLogPrintf("SCICLIENT: Sciclient_pmModuleClkRequest success\n"); \
        } \
    } \
} while(0)

#define QUERY_CLOCK_FREQ(MOD, CLK, FREQ) do { \
    int32_t status = 0; \
    uint64_t freq; \
    if(status == 0) { \
        if(APP_DEBUG_SCICLIENT) \
            appLogPrintf("SCICLIENT: Sciclient_pmQueryModuleClkFreq module=%u clk=%u freq=%u%06u\n", MOD, CLK, (uint32_t)(FREQ / 1000000), (uint32_t)(FREQ % 1000000)); \
        status = Sciclient_pmQueryModuleClkFreq(MOD, CLK, FREQ, &freq, SCICLIENT_SERVICE_WAIT_FOREVER); \
        if(status != 0) appLogPrintf("SCICLIENT: ERROR: Sciclient_pmQueryModuleClkFreq failed\n"); \
        else \
        { \
            if(APP_DEBUG_SCICLIENT) \
                appLogPrintf("SCICLIENT: Sciclient_pmQueryModuleClkFreq freq=%u%06u\n", (uint32_t)(freq / 1000000), (uint32_t)(freq % 1000000)); \
        } \
    } \
} while(0)

#define SET_CLOCK_FREQ(MOD, CLK, FREQ) do { \
    int32_t status = 0; \
    if(status == 0) { \
        if(APP_DEBUG_SCICLIENT) \
            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq module=%u clk=%u freq=%u%06u\n", MOD, CLK, (uint32_t)(FREQ / 1000000), (uint32_t)(FREQ % 1000000)); \
        status = Sciclient_pmSetModuleClkFreq(MOD, CLK, FREQ, 0, SCICLIENT_SERVICE_WAIT_FOREVER); \
        if(status != 0) appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleClkFreq failed\n"); \
        else \
        { \
            if(APP_DEBUG_SCICLIENT) \
                appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq success\n"); \
        } \
    } \
} while(0)

#define SET_CLOCK_FREQ_ALLOW_CHANGE(MOD, CLK, FREQ) do { \
    int32_t status = 0; \
    if(status == 0) { \
        if(APP_DEBUG_SCICLIENT) \
            appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq module=%u clk=%u freq=%u%06u\n", MOD, CLK, (uint32_t)(FREQ / 1000000), (uint32_t)(FREQ % 1000000)); \
        status = Sciclient_pmSetModuleClkFreq(MOD, CLK, FREQ, TISCI_MSG_FLAG_CLOCK_ALLOW_FREQ_CHANGE, SCICLIENT_SERVICE_WAIT_FOREVER); \
        if(status != 0) appLogPrintf("SCICLIENT: ERROR: Sciclient_pmSetModuleClkFreq failed\n"); \
        else \
        { \
            if(APP_DEBUG_SCICLIENT) \
                appLogPrintf("SCICLIENT: Sciclient_pmSetModuleClkFreq success\n"); \
        } \
    } \
} while(0)

#endif
