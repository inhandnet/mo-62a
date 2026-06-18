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

#ifndef APP_UDMA_UTILS_H_
#define APP_UDMA_UTILS_H_

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

#ifndef x86_64
#include "app_udma.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/** \brief max number of ND copy channels one can open */
#define APP_UDMA_ND_CHANNELS_MAX        (16U)

/** \brief App UDMA channel handle */
typedef void *app_udma_ch_handle_t;

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/** \brief Parameter to use when creating a logical DMA channel
 *        To avoid compatibility issues, user should called
 *        appUdmaCreatePrms_Init() to set default values for all params
 *        in this structure
 */
typedef struct
{
    uint32_t    enable_intr;
    /**< Enable interrupt based DMA completion waiting */
    uint32_t    use_dru;
    /**< UTRUE - Use DRU channel. UFALSE - Use DMA blockcopy channel */
    uint32_t    use_ring;
    /**< UTRUE - Use Ring based UDMA. UFALSE - Use without ring */
    uint32_t    use_nd_copy;
    /**< UTRUE - Use channel for ND copy. UFALSE - Use channel for normal blockcopy */
} app_udma_create_prms_t;

/** \brief 1D transfer request parameters */
typedef struct
{
    uint64_t    dest_addr;
    /**< Physical address of destination buffer.
     *   For max efficiency recommended to be 64-byte aligned */
    uint64_t    src_addr;
    /**< Physical address of a source buffer.
     *   For max efficiency recommended to be 64-byte aligned */
    uint32_t    length;
    /**< Number of bytes to be transferred */
} app_udma_copy_1d_prms_t;

/** \brief 2D transfer request parameters */
typedef struct
{
    uint16_t    width;
    /**< Width of data to fill in destination buffer - in bytes
     *   For max efficiency recommended to be 64-byte aligned */
    uint16_t    height;
    /**< Height of data to fill in destination buffer - in unit of lines */
    uint64_t    dest_addr;
    /**< Physical address of destination buffer.
     *   For max efficiency recommended to be 64-byte aligned */
    uint32_t    dest_pitch;
    /**< Pitch in bytes of the destination buffer.
     *   For max efficiency recommended to be 64-byte aligned */

    uint64_t    src_addr;
    /**<
     *   For appUdmaCopy2D(),
     *   Physical address of a source buffer
     *   - For max efficiency recommended to be 64-byte aligned
     *
     *   For appUdmaFill2D(),
     *   Physical address of a source line buffer
     *   - User should make sure the buffer pointed to by src_addr
     *     can hold one line of size 'width' bytes
     *   - User should pre-fill this buffer with the value he wants to fill
     *     across the buffer
     *   - For max efficiency recommended to be 64-byte aligned */
    uint32_t    src_pitch;
    /**<
     *   For appUdmaCopy2D(),
     *   Pitch in bytes of the source buffer
     *   - For max efficiency recommended to be 64-byte aligned
     *
     *   For appUdmaFill2D(),
     *   - NOT USED */
} app_udma_copy_2d_prms_t;

/** \brief ND transfer request parameters */
typedef struct
{
    uint32_t    copy_mode;
    /**< Mode of copy,
     * If set to 0 - Halt DMA after entire set is transferred
     * If set to 1 - Halt DMA after every icnt1 transfer, re-trigger to complete
     * If set to 2 - Halt DMA after every icnt2 transfer, re-trigger to complete
     * If set to 3 - Halt DMA after every icnt3 transfer, re-trigger to complete */

    uint32_t    eltype;
    /**< Type of element,
     * If set to 0 - Default to 1 byte per element
     * If set to 1 - 1 byte per element
     * If set to 2 - 2 bytes per element
     * If set to 3 - 3 bytes per element
     * If set to 4 - 4 bytes per element */

    uint16_t    icnt0;
    /**< Iteration count 0 of the transfer block,
     * Eg. width  */
    uint16_t    icnt1;
    /**< Iteration count 1 of the transfer block,
     * Eg. height */
    uint16_t    icnt2;
    /**< Iteration count 2 of the transfer block,
     * Eg. number of blocks of size (width * height)  */
    uint16_t    icnt3;
    /**< Iteration count 3 of the transfer block,
     * Eg. number of sets of size (width * height * blocks) */

    int32_t     dim1;
    /**< Dimension 1 of the transfer block in bytes,
     * Eg. stride from one line to the next  */
    int32_t     dim2;
    /**< Dimension 2 of the transfer block in bytes,
     * Eg. stride from one block to the next  */
    int32_t     dim3;
    /**< Dimension 3 of the transfer block in bytes,
     * Eg. stride from one set to the next  */

    uint16_t    dicnt0;
    /**< Iteration count 0 of the transfer block,
     * Eg. destination width  */
    uint16_t    dicnt1;
    /**< Iteration count 1 of the transfer block,
     * Eg. destination height */
    uint16_t    dicnt2;
    /**< Iteration count 2 of the transfer block,
     * Eg. destination block size (width * height)  */
    uint16_t    dicnt3;
    /**< Iteration count 3 of the transfer block,
     * Eg. destination set size (width * height * blocks) */

    int32_t     ddim1;
    /**< Dimension 1 of the transfer block in bytes,
     * Eg. stride at destination from one line to the next  */
    int32_t     ddim2;
    /**< Dimension 2 of the transfer block in bytes,
     * Eg. stride at destination from one block to the next  */
    int32_t     ddim3;
    /**< Dimension 3 of the transfer block in bytes,
     * Eg. stride at destination from one set to the next  */

    uint64_t    dest_addr;
    /**< Physical address of destination buffer.
     *   For max efficiency recommended to be 64-byte aligned */

    uint64_t    src_addr;
    /**<
     *   Physical address of a source buffer
     *   - For max efficiency recommended to be 64-byte aligned */
} app_udma_copy_nd_prms_t;

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/**
 *  \brief DMA copy init function.
 *
 *   This should be called before calling other function.
 *   This function creates the default DMA channel required for API which
 *   passes NULL as the channel object.
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopyInit(void);

/**
 *  \brief DMA copy deinit function.
 *
 *   This function frees up the default DMA channel allocated during init.
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopyDeinit(void);

/**
 *  \brief DMA copy create channel function.
 *
 *   This function allocates a memcpy DMA channel and initialize the DMA
 *   channel for data transfer.
 *   This is an optional function which can be used to create more DMA channel
 *   apart from the default DMA channel allocated during init.
 *
 *  \param prms     [IN]     Pointer to channel params. Caller should initialize
 *                           this structure using #appUdmaCreatePrms_Init
 *                           before modifying the content to ensure backward
 *                           compatibility.
 *
 *  \return  Pointer to channel object which will get initialized and contexts
 *   are stored. The channel object memory is allocated by this call and
 *   returned. This API returns NULL incase of failure.
 */
app_udma_ch_handle_t appUdmaCopyCreate(const app_udma_create_prms_t *prms);

/**
 *  \brief DMA copy delete channel function.
 *
 *   This function frees-up DMA channel and all associated resources.
 *
 *  \param ch_handle [IN]   Pointer to channel object already created through
 *                          #appUdmaCopyCreate
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopyDelete(app_udma_ch_handle_t ch_handle);

/**
 *  \brief DMA copy 1D DMA copy function.
 *
 *   This function will initiate a 1D DMA copy operation.
 *
 *  \param ch_handle [IN]   Pointer to channel object already created through
 *                          #appUdmaCopyCreate. If NULL is passed, the API
 *                          will use the default handle created during init.
 *  \param prms_1d   [IN]   Pointer to 1D transfer params. This can't be NULL.
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopy1D(
    app_udma_ch_handle_t ch_handle,
    const app_udma_copy_1d_prms_t *prms_1d);

/**
 *  \brief DMA copy 2D DMA copy function.
 *
 *   This function will initiate a 2D DMA copy operation.
 *
 *  \param ch_handle [IN]   Pointer to channel object already created through
 *                          #appUdmaCopyCreate. If NULL is passed, the API
 *                          will use the default handle created during init.
 *  \param prms_2d   [IN]   Pointer to 2D transfer params. This can't be NULL.
 *                          This could be an array of params depending on
 *                          num_transfers parameter
 *  \param num_transfers [IN] The API will iterate through this many number of
 *                            transfers.
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopy2D(
    app_udma_ch_handle_t ch_handle,
    const app_udma_copy_2d_prms_t *prms_2d,
    uint32_t num_transfers);

/**
 *  \brief DMA copy 2D DMA memfill function.
 *
 *   This function will initiate a 2D DMA memfill operation.
 *   The only difference between this and #appUdmaCopy2D is that this API
 *   assumes that the source buffer is just oneline worth of data and will
 *   setup the DMA parameter such that the DMA keeps reading the same one line
 *   source buffer and fill the destination buffer
 *
 *  \param ch_handle [IN]   Pointer to channel object already created through
 *                          #appUdmaCopyCreate. If NULL is passed, the API
 *                          will use the default handle created during init.
 *  \param prms_2d [IN]     Pointer to 2D transfer params. This can't be NULL.
 *                          This could be an array of params depending on
 *                          num_transfers parameter
 *  \param num_transfers [IN] The API will iterate through this many number of
 *                            transfers.
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaFill2D(
    app_udma_ch_handle_t ch_handle,
    const app_udma_copy_2d_prms_t *prms_2d,
    uint32_t num_transfers);

/**
 *  \brief Return handle to ND copy channel, if not already created, CH is created here
 *
 *  \param  ch_idx [IN] channel index to use, upto APP_UDMA_ND_CHANNELS_MAX are allowed
 *
 *  \return  Pointer to channel object, if unable to create CH then NULL is returned;
 */
app_udma_ch_handle_t appUdmaCopyNDGetHandle(uint32_t ch_idx);

/**
 *  \brief Releases reference to channel handle, if found to be last reference deletes the handle
 *
 *  \param  ch_idx [IN] channel index to use, upto APP_UDMA_ND_CHANNELS_MAX are allowed
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopyNDReleaseHandle(uint32_t ch_idx);

/**
 *  \brief DMA copy ND init function.
 *
 *   This function will setup a N-Dimension (upto 4) DMA copy operation
 *   based on "prms_nd" transfer params.
 *
 *  \param ch_handle [IN]   Pointer to channel object already created through
 *                          #appUdmaCopyCreate. If NULL is passed, the API
 *                          will use the default handle created during init.
 *  \param prms_nd   [IN]   Pointer to ND transfer params. This can't be NULL.
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopyNDInit(
    app_udma_ch_handle_t ch_handle,
    const app_udma_copy_nd_prms_t *prms_nd);

/**
 *  \brief DMA copy ND de-init function.
 *
 *   This function will setup a N-Dimension (upto 4) DMA copy operation
 *   based on "prms_nd" transfer params.
 *
 *  \param ch_handle [IN]   Pointer to channel object already created through
 *                          #appUdmaCopyCreate. If NULL is passed, the API
 *                          will use the default handle created during init.
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopyNDDeinit(
    app_udma_ch_handle_t ch_handle);

/**
 *  \brief DMA copy ND trigger function.
 *
 *   This function will initiate a N-Dimension (upto 4) DMA copy operation
 *   based on "copy_mode" option set in prms_nd transfer params.
 *
 *  \param ch_handle [IN]   Pointer to channel object already created through
 *                          #appUdmaCopyCreate. If NULL is passed, the API
 *                          will use the default handle created during init.
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopyNDTrigger(
    app_udma_ch_handle_t ch_handle);


/**
 *  \brief DMA copy ND wait function.
 *
 *   This function will wait for a N-Dimension (upto 4) DMA copy operation
 *   based on "copy_mode" option set in prms_nd transfer params.
 *
 *  \param ch_handle [IN]   Pointer to channel object already created through
 *                          #appUdmaCopyCreate. If NULL is passed, the API
 *                          will use the default handle created during init.
 *
 *  \return  0 incase of success else returns failure code
 */
int32_t appUdmaCopyNDWait(
    app_udma_ch_handle_t ch_handle);


/**
 *  \brief Prints params values
 */
void appUdmaCopyNDPrmsPrint(app_udma_copy_nd_prms_t *prm, char *name);

/**
 *  \brief Set defaults for app_udma_create_prms_t structure
 *
 *  \param prms [OUT] Parameters to init to default value
 */
static inline void appUdmaCreatePrms_Init(app_udma_create_prms_t *prms);
/**
 *  \brief Set defaults for app_udma_copy_1d_prms_t structure
 *
 *  \param prms_1d [OUT] Parameters to init to default value
 */
static inline void appUdmaCopy1DPrms_Init(app_udma_copy_1d_prms_t *prms_1d);
/**
 *  \brief Set defaults for app_udma_copy_2d_prms_t structure
 *
 *  \param prms_2d [OUT] Parameters to init to default value
 */
static inline void appUdmaCopy2DPrms_Init(app_udma_copy_2d_prms_t *prms_2d);
/**
 *  \brief Set defaults for app_udma_copy_nd_prms_t structure
 *
 *  \param prms_nd [OUT] Parameters to init to default value
 */
static inline void appUdmaCopyNDPrms_Init(app_udma_copy_nd_prms_t *prms_nd);

/* ========================================================================== */
/*                       Static Function Definitions                          */
/* ========================================================================== */

static inline void appUdmaCreatePrms_Init(app_udma_create_prms_t *prms)
{
    if(NULL != prms)
    {
        /* Interrupt mode not yet supported for C7x - use polling */
#if defined(C7X_FAMILY)
        prms->enable_intr = 0;
#else
        prms->enable_intr = 1;
#endif
        prms->use_dru     = 0;
        prms->use_ring    = 1;
        prms->use_nd_copy = 0; /* Transfer to completion */
    }
}

static inline void appUdmaCopy1DPrms_Init(app_udma_copy_1d_prms_t *prms_1d)
{
    if(NULL != prms_1d)
    {
        prms_1d->dest_addr  = 0U;
        prms_1d->src_addr   = 0U;
        prms_1d->length     = 0U;
    }
}

static inline void appUdmaCopy2DPrms_Init(app_udma_copy_2d_prms_t *prms_2d)
{
    if(NULL != prms_2d)
    {
        prms_2d->width      = 0U;
        prms_2d->height     = 0U;
        prms_2d->dest_addr  = 0U;
        prms_2d->dest_pitch = 0U;
        prms_2d->src_addr   = 0U;
        prms_2d->src_pitch  = 0U;
    }
}

static inline void appUdmaCopyNDPrms_Init(app_udma_copy_nd_prms_t *prms_nd)
{
    if(NULL != prms_nd)
    {
        prms_nd->copy_mode  = 2;
        prms_nd->icnt0      = 0U;
        prms_nd->icnt1      = 0U;
        prms_nd->icnt2      = 0U;
        prms_nd->icnt3      = 0U;
        prms_nd->dim1       = 0U;
        prms_nd->dim2       = 0U;
        prms_nd->dim3       = 0U;
        prms_nd->dest_addr  = 0U;
        prms_nd->src_addr   = 0U;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* @} */

#endif
