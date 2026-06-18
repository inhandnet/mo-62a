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

#ifndef APP_UTILS_MISC_H_
#define APP_UTILS_MISC_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \defgroup group_vision_apps_utils_misc Miscellaneous utility APIs (RTOS only)
 *
 * \brief This section contains miscellaneous utility APIs
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

#include <stdint.h>

typedef struct
{
    uint32_t enable_hdmi;
    uint32_t enable_i2c;
} app_pinmux_cfg_t;


/**
 * \brief Switch C7x from secure supervisor to non-secure supervisor
 *
 * NOTE, this API must be called after MMU and Cache init
 */
uint64_t appC7xSecSupv2NonSecSupv ( void );


/**
 * \brief Init CLEC so that C7x in non-secure mode can program it
 *
 * This also sets defaults for DRU input events to what TIDL needs
 *
 * This API MUST be called before switching C7x to secure mode
 */
void appC7xClecInitForNonSecAccess(void);

/**
 * \brief API to set to DLFO bit in ACTRL register of R5F
 *
 * This API uses assembly instruction to set DLFO bit in ACTRL register
 * of R5F.
 * This should be called from the Core reset callback.
 *
 */
void appUtilsSetDLFOBitInACTRLReg(void);

/**
 * \brief API to set to set pinmux required for running basic demos in Vision Apps
 *
 * It internally uses Board API to configure Pinmux.
 *
 */
void appSetPinmux(app_pinmux_cfg_t *cfg);

/**
 * \brief Inline Function to initialize Pinmux config to default value
 *
 */
void appPinMuxCfgSetDefault(app_pinmux_cfg_t *cfg);

/**
 * \brief API to print the CPU Frequency in Hz
 *
 */
int32_t appUtilsPrintCpuHz(void);

/**
 * \brief API call OS-specific init API's when creating tasks
 *
 */
void appUtilsTaskInit(void);

#ifdef __cplusplus
}
#endif

/* @} */

#endif /* APP_UTILS_MISC_H_ */

