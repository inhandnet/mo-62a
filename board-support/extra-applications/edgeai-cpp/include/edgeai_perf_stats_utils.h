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
#ifndef _EDGEAI_PERF_STATS_UTILS_
#define _EDGEAI_PERF_STATS_UTILS_

#if defined(SOC_J721E)
#define NUM_THERMAL_ZONE 5
#elif defined(SOC_J721S2) || defined(SOC_J784S4) || defined(SOC_J742S2)
#define NUM_THERMAL_ZONE 7
#elif defined(SOC_AM62A) || defined(SOC_AM62X) || defined(SOC_AM62P) || defined(SOC_J722S)
#define NUM_THERMAL_ZONE 3
#else
#define NUM_THERMAL_ZONE 0
#endif

/* Standard headers. */
#include <stdint.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  ################################
    UTILS FOR PERFORMANCE STATISTICS
    ################################
*/

typedef struct {

    uint64_t total_time;
    uint64_t busy_time;
    uint64_t irq_time;
    uint64_t softirq_time;
    uint32_t cpu_load; /**< divide by 100 to get load in units of xx.xx % */
    uint32_t hwi_load; /**< divide by 100 to get load in units of xx.xx % */
    uint32_t swi_load; /**< divide by 100 to get load in units of xx.xx % */

} perfStatsCpuLoad;

/**
* Utility Function to calculate cpu loading
*
* @param cpuLoad       Pointer to perfStatsCpuLoad structure
*/
void perfStatsCpuLoadCalc(perfStatsCpuLoad *cpuLoad);

/**
* Utility Function to Initialize cpuLoad stats
*/
void perfStatsCpuStatsInit(perfStatsCpuLoad *cpu_load);

/**
* Utility Function to Print CPU stats
*/
void perfStatsCpuStatsPrint(perfStatsCpuLoad *cpu_load);

/**
* Utility Function to reset perfStatsCpuLoad struct members
*
* @param cpuLoad       Pointer to perfStatsCpuLoad structure
*/
void perfStatsResetCpuLoadCalc(perfStatsCpuLoad *cpuLoad);

/**
 * \brief DDR BW information
 *
 * note, this information is retrived from MCU2-1
 *       EMIF counters are used to sample read, write access every 1ms periodicity
 */
typedef struct {

    uint32_t read_bw_avg;   /**< avg bytes read per second, in units of MB/s */
    uint32_t write_bw_avg;  /**< avg bytes written per second, in units of in MB/s */
    uint32_t read_bw_peak;  /**< peak bytes read in a sampling period, in units of MB/s */
    uint32_t write_bw_peak; /**< peak bytes read in a sampling period, in units of MB/s */
    uint32_t total_available_bw; /**< theoritical bw available to system, in units of MB/s */

    uint32_t counter0_total;  /**< sum total of counter0 values aggregated over time as defined by APP_PERF_SNAPSHOT_WINDOW_WIDTH */
    uint32_t counter1_total;  /**< sum total of counter1 values aggregated over time as defined by APP_PERF_SNAPSHOT_WINDOW_WIDTH */
    uint32_t counter2_total;  /**< sum total of counter2 values aggregated over time as defined by APP_PERF_SNAPSHOT_WINDOW_WIDTH */
    uint32_t counter3_total;  /**< sum total of counter3 values aggregated over time as defined by APP_PERF_SNAPSHOT_WINDOW_WIDTH */

} perf_stats_ddr_stats_t;

/**
* Utility Function to Get DDR stats
*
* @returns DDR Stats
*/
perf_stats_ddr_stats_t *perfStatsDdrStatsGet();

/**
* Utility Function to Print DDR stats
*/
void perfStatsDdrStatsPrintAll();

/**
* Utility Function to Clear DDR Stats
*/
void perfStatsResetDdrLoadCalcAll();

typedef struct {

    float thermal_zone_temp[NUM_THERMAL_ZONE];
    char  thermal_zone_name[NUM_THERMAL_ZONE][128];

} perfStatsSOCTemp;

/**
* Utility Function to initialize perfStatsSOCTemp
*
* @param socTemp       Pointer to perfStatsSOCTemp structure
*/
void perfStatsSocTempInit(perfStatsSOCTemp *socTemp);

/**
* Utility Function to get SOC temperature
*
* @param socTemp       Pointer to perfStatsSOCTemp structure
*/
void perfStatsSocTempGet(perfStatsSOCTemp *socTemp);

#ifdef __cplusplus
}
#endif

#endif
