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
#ifndef _EDGEAI_OVERLAY_PERF_STATS_UTILS_
#define _EDGEAI_OVERLAY_PERF_STATS_UTILS_

#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
#if defined(SOC_AM62A) || defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
#include <utils/app_init/include/app_init.h>
#include <utils/perf_stats/include/app_perf_stats.h>
#include <utils/perf_stats/src/app_perf_stats_priv.h>
#include <utils/ipc/include/app_ipc.h>
#include <utils/remote_service/include/app_remote_service.h>
#endif
#endif

#include <edgeai_nv12_drawing_utils.h>
#include <edgeai_perf_stats_utils.h>

#include <stdbool.h>
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OVERLAY_TYPE_NONE  (0)
#define OVERLAY_TYPE_GRAPH (1)
#define OVERLAY_TYPE_TEXT  (2)

#define MAX_GRAPHS (32)

#define DEFAULT_OVERLAY_WIDTH (1920)
#define DEFAULT_OVERLAY_HEIGHT (200)

#define DEFAULT_GRAPH_WIDTH (60)
#define DEFAULT_GRAPH_HEIGHT (150)

typedef struct {
    perfStatsCpuLoad            cpu_load;
    perf_stats_ddr_stats_t      ddr_load;
#if defined(TARGET_CPU_A72) || defined(TARGET_CPU_A53)
#if defined(SOC_AM62A) || defined(SOC_J721E) || defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J722S) || defined(SOC_J742S2)
    app_perf_stats_cpu_load_t   cpu_loads[APP_IPC_CPU_MAX];
    app_perf_stats_hwa_stats_t  hwa_loads[4];
    uint32_t                    hwa_count;
#endif
#endif
    perfStatsSOCTemp            soc_temp;
    uint32_t                    fps;
    uint32_t                    stats_count; 
} Stats;

typedef struct {
    uint8_t         *imgYPtr;
    uint8_t         *imgUVPtr;
    uint32_t        imgWidth;
    uint32_t        imgHeight;
    uint32_t        xPos;
    uint32_t        yPos;
    uint32_t        width;
    uint32_t        height;
    YUVColor        backgroundColor;
    YUVColor        colorWhite;
    YUVColor        colorBlack;
    YUVColor        colorGrey;
    YUVColor        colorRed;
    YUVColor        colorYellow;
    YUVColor        colorGreen;
    YUVColor        colorPurple;
    YUVColor        colorPink;
    YUVColor        colorOrange;
    BarGraph        graphs[MAX_GRAPHS];
    bool            show_fps;
    bool            show_temp;
} Overlay;

typedef struct {
    Stats           stats;
    uint32_t        updateTimeuS;
    struct timeval  prevTime;
    uint32_t        frameCount;
    bool            firstCall;
    uint8_t         overlayType;
    Overlay         overlay;
    uint8_t         numInstances;
} EdgeAIPerfStats;

void initialize_edgeai_perf_stats(EdgeAIPerfStats *perf_stats);
int32_t update_edgeai_perf_stats(EdgeAIPerfStats *perf_stats);


#ifdef __cplusplus
}
#endif

#endif
