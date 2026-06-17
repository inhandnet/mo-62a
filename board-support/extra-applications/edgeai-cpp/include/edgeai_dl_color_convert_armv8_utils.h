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
#ifndef _EDGEAI_DL_COLOR_CONVERT_ARMV8_UTILS
#define _EDGEAI_DL_COLOR_CONVERT_ARMV8_UTILS

#include <edgeai_image_params.h>

#ifdef __cplusplus
extern "C" {
#endif

// #define DL_COLOR_CONVERT_CHECK_PARAMS

#define COLOR_SPACE_BT601_525 0x1     /* Defines BT601_525 color space */
#define COLOR_SPACE_BT601_625 0x2     /* Defines BT601_625 color space */

/*  ################################
    UTILS FOR DL_COLOR_CONVERT_ARMV8
    ################################
*/

/**
* Utility Function for color format conversion from RGB to NV12
*
* @param src        Source data pointer for RGB
* @param src_prms   Source parameters     
* @param dst0       Destination pointer for Y plane
* @param dst0_prms  Destination parameters for Y plane
* @param dst1       Destination pointer for UV plane
* @param dst1_prms  Destination parameters for UV plane
*/
int32_t colorConvert_RGBtoNV12_i8u_o8u_armv8
(
    uint8_t src[],
    bufParams2D_t *src_prms,
    uint8_t dst0[],
    bufParams2D_t * dst0_prms,
    uint8_t dst1[],
    bufParams2D_t * dst1_prms
);

/**
* Utility Function for color format conversion from NV12/NV21 to RGB
*
* @param src0       Source data pointer for Y plane
* @param src0_prms  Source parameters for Y plane
* @param src1       Source data pointer for UV/VU plane
* @param src1_prms  Source parameters for UV/VU plane   
* @param dst        Destination pointer for RGB plane
* @param dst_prms   Destination parameters for RGB plane
* @param u_pix      Position of U pixel in UV/VU plane.
*                   u_pix = 0 for NV12 and 1 for NV21
* @param src_space  Color space for source i.e. BT601_525/BT601_625/OTHERS
*/
int32_t colorConvert_NVXXtoRGB_i8u_o8u_armv8
(
    uint8_t src0[],
    bufParams2D_t * src0_prms,
    uint8_t src1[],
    bufParams2D_t * src1_prms,
    uint8_t dst[],
    bufParams2D_t * dst_prms,
    uint8_t u_pix,
    uint8_t src_space
);

/**
* Utility Function for color format conversion from NV12/NV21 to I420
*
* @param src0       Source data pointer for Y plane
* @param src0_prms  Source parameters for Y plane
* @param src1       Source data pointer for UV/VU plane
* @param src1_prms  Source parameters for UV/VU plane
* @param dst0       Destination pointer for Y plane
* @param dst0_prms  Destination parameters for Y plane
* @param dst1       Destination pointer for U plane
* @param dst1_prms  Destination parameters for U plane
* @param dst2       Destination pointer for V plane
* @param dst2_prms  Destination parameters for V plane
* @param u_pix      Position of U pixel in UV/VU plane.
*                   u_pix = 0 for NV12 and 1 for NV21
*/
int32_t colorConvert_NVXXtoIYUV_i8u_o8u_armv8
(
    uint8_t src0[],
    bufParams2D_t *src0_prms,
    uint8_t src1[],
    bufParams2D_t *src1_prms,
    uint8_t dst0[],
    bufParams2D_t *dst0_prms,
    uint8_t dst1[],
    bufParams2D_t *dst1_prms,
    uint8_t dst2[],
    bufParams2D_t *dst2_prms,
    uint8_t u_pix
);

/**
* Utility Function for color format conversion from I420 to NV12
*
* @param src0       Source data pointer for Y plane
* @param src0_prms  Source parameters for Y plane
* @param src1       Source data pointer for U plane
* @param src1_prms  Source parameters for U plane
* @param src2       Source data pointer for V plane
* @param src2_prms  Source parameters for V plane
* @param dst0       Destination pointer for Y plane
* @param dst0_prms  Destination parameters for Y plane
* @param dst1       Destination pointer for UV plane
* @param dst1_prms  Destination parameters for UV plane
*/
int32_t colorConvert_IYUVtoNV12_i8u_o8u_armv8
(
    uint8_t src0[],
    bufParams2D_t * src0_prms,
    uint8_t src1[],
    bufParams2D_t * src1_prms,
    uint8_t src2[],
    bufParams2D_t * src2_prms,
    uint8_t dst0[],
    bufParams2D_t * dst0_prms,
    uint8_t dst1[],
    bufParams2D_t * dst1_prms
);

/**
* Utility Function for color format conversion from UYVY/YUY2 to NV12
*
* @param src       Source data pointer for UYVY/YUY2 plane
* @param src_prms  Source parameters for UYVY/YUY2 plane
* @param dst0      Destination pointer for Y plane
* @param dst0_prms Destination parameters for Y plane
* @param dst1      Destination pointer for UV plane
* @param dst1_prms Destination parameters for UV plane
* @param y_pix     y_pix = 1 for UYVY and y_pix = 0 for YUY2
*/
int32_t colorConvert_YUVXtoNV12_i8u_o8u_armv8
(
    uint8_t src[],
    bufParams2D_t * src_prms,
    uint8_t dst0[],
    bufParams2D_t * dst0_prms,
    uint8_t dst1[],
    bufParams2D_t * dst1_prms,
    uint8_t y_pix
);

/**
* Utility Function for color format conversion from U8 to NV12
*
* @param src        Source data pointer for U8
* @param src_prms   Source parameters
* @param dst0       Destination pointer for Y plane
* @param dst0_prms  Destination parameters for Y plane
* @param dst1       Destination pointer for UV plane
* @param dst1_prms  Destination parameters for UV plane
*/
int32_t colorConvert_U8toNV12_i8u_o8u_armv8
(
    uint8_t src[],
    bufParams2D_t *src_prms,
    uint8_t dst0[],
    bufParams2D_t * dst0_prms,
    uint8_t dst1[],
    bufParams2D_t * dst1_prms
);

#ifdef __cplusplus
}
#endif

#endif
