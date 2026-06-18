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

#ifndef APP_UDMA_H_
#define APP_UDMA_H_

/**
 * \defgroup group_vision_apps_utils_udma UDMA initialization APIs (RTOS only)
 *
 * \brief This section contains APIs for UDMA initialization
 *
 *        Few utility APIs to copy/fill frames are implemented in this library
 *        using the UDMA LLD for convenience of application code.
 *
 *        For algorithms it is recommended to use the UDMA LLD APIs directly
 *        or UDMA DMA copy API present in PDK to avoid additional layer of
 *        software overhead.
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

#include <stdint.h>
#include <stdlib.h>

#if defined(MCU_PLUS_SDK) && !defined(QNX)
#if defined(HOST_EMULATION)
#include <drivers/dmautils/udma_standalone/udma.h>
#else
#include <drivers/udma.h>
#include <drivers/udma/udma_priv.h>
#endif
#else
#include <ti/drv/udma/udma.h>
#endif

#if !defined(SOC_AM62A)
#include "app_udma_utils.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief UDMA init parameters */
typedef struct
{
    Udma_VirtToPhyFxn       virtToPhyFxn;
    /**< If not NULL, this function will be called to convert virtual address
     *   to physical address to be provided to UDMA.
     *   If NULL, the driver will assume a one-one mapping.
     *
     *   Note: The init fxn will initialize this to the default one-one map
     *   function #Udma_defaultVirtToPhyFxn
     */
} app_udma_init_prms_t;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

#if !(defined(SOC_AM62A) || defined(SOC_J722S))
#if defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)

/**
 *  \brief Performs initializations needed for UDMA BCDMA instance
 *         Required for CSIRX/CSITX on J721S2
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCsirxCsitxInit(void);

/**
 *  \brief Performs de-initializations needed for UDMA BCDMA instance
 *         Required for CSIRX/CSITX on J721S2
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCsirxCsitxDeInit(void);

#endif

/**
 *  \brief Provides global handle to UDMA Obj BCDMA instance
 *         Required for CSIRX/CSITX on J721S2
 *
 *  \return  0 incase of success else returns failure code
 */
void *appUdmaCsirxCsitxGetObj(void);
#endif

/**
 *  \brief Sets default UDMA init parameters
 *
 *  \param prms   [IN]   Pointer to UDMA init params
 *
 */
void appUdmaInitPrmSetDefault(app_udma_init_prms_t *prm);

/**
 *  \brief Performs initializations needed for UDMA
 *
 *  \param prms   [IN]   Pointer to UDMA init params
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaInit(const app_udma_init_prms_t *prms);

/**
 *  \brief Performs de-initializations needed for UDMA
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaDeInit(void);

/**
 *  \brief Provides global handle to UDMA Obj
 *
 *  \return  0 incase of success else returns failure code
 */
void *appUdmaGetObj(void);

/**
 *  \brief DMA copy test function.
 *
 *   This function tests basic features of DMA copy. This could be called
 *   from vision apps test function. And not to be used by other application
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaTest(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* @} */

#endif
