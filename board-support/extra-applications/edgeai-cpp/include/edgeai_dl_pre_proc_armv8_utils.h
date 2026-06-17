/*
 *
 * Copyright (c) 2023 Texas Instruments Incorporated
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
#ifndef _EDGEAI_DL_PRE_PROC_ARMV8_UTILS
#define _EDGEAI_DL_PRE_PROC_ARMV8_UTILS

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_TENSOR_DIMS (4)

/* Supported tensor formats in dl-pre-proc */
#define DL_PRE_PROC_ARMV8_TENSOR_FORMAT_RGB   (0)
#define DL_PRE_PROC_ARMV8_TENSOR_FORMAT_BGR   (1)

/* Supported channel ordering in dl-pre-proc */
#define DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NCHW   (0)
#define DL_PRE_PROC_ARMV8_CHANNEL_ORDER_NHWC   (1)


/*  ###########################
    UTILS FOR DL_PRE_PROC_ARMV8
    ###########################
*/

typedef struct
{
    /* Channel ordering, 0-NCHW, 1-NHWC */
    uint32_t channel_order;

    /* Tensor format, 0-RGB, 1-BGR */
    uint32_t tensor_format;

    /*
    These are VX data type enums used for below
    mentioned data formats. Corresponding values
    must be used for using that particular data
    type.
    VX_TYPE_INT8            = 0x002
    VX_TYPE_UINT8           = 0x003
    VX_TYPE_INT16           = 0x004
    VX_TYPE_UINT16          = 0x005
    VX_TYPE_INT32           = 0x006
    VX_TYPE_UINT32          = 0x007
    VX_TYPE_FLOAT32         = 0x00A
    */
    uint32_t tensor_data_type;

    uint32_t input_width;
    uint32_t input_height;
    uint32_t in_stride_y;

    /* Input image data pointers of two planes */
    void*    in_img_target_ptr[2];

    uint32_t output_dimensions[MAX_TENSOR_DIMS];
    void*    out_tensor_target_ptr;

    float    mean[3];
    float    scale[3];

}dlPreProcessImageParams;

void dlPreProcess_RGB_image
(
    dlPreProcessImageParams *prms
);

void dlPreProcess_NV12_image
(
    dlPreProcessImageParams *prms
);

#ifdef __cplusplus
}
#endif

#endif
