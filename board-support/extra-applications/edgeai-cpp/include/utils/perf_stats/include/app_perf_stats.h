/*
 *
 * Copyright (c) 2019 Texas Instruments Incorporated
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

#ifndef __APP_PERF_STATS_H__
#define __APP_PERF_STATS_H__

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <utils/mem/include/app_mem.h>

/**
 * \defgroup group_vision_apps_utils_perf_stats Performance statistics reporting APIs
 *
 * \brief These APIs allows user to get performance information of RTOS based remote cores
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

/** \brief HWA ID */
typedef enum {

    APP_PERF_HWA_VPAC1_VISS,
    APP_PERF_HWA_VPAC1_LDC,
    APP_PERF_HWA_VPAC1_NF,
    APP_PERF_HWA_VPAC1_MSC0,
    APP_PERF_HWA_VPAC1_MSC1,
    APP_PERF_HWA_DOF,
    APP_PERF_HWA_SDE,
    APP_PERF_HWA_GPU,
    #if defined(SOC_J784S4) || defined(SOC_J742S2)
    APP_PERF_HWA_VPAC2_VISS,
    APP_PERF_HWA_VPAC2_LDC,
    APP_PERF_HWA_VPAC2_NF,
    APP_PERF_HWA_VPAC2_MSC0,
    APP_PERF_HWA_VPAC2_MSC1,
    #endif
    APP_PERF_HWA_MAX

} app_perf_hwa_id_t;


/** \brief Max size of task name string */
#define APP_PERF_STATS_TASK_NAME_MAX (12u)

/** \brief Max number of tasks whoose information can be retrived */
#define APP_PERF_STATS_TASK_MAX (24u)

/** \brief Max size of performance point name string */
#define APP_PERF_POINT_NAME_MAX     (16u)

/** \brief Max file name size for output */
#define APP_PERF_POINT_MAX_FILENAME     (256u)

/** \brief Max file line size for output */
#define APP_PERF_MAX_LINE_SIZE     (1024u)

/**
 * \brief Summary of CPU load
 */
typedef struct {

    uint32_t cpu_load; /**< divide by 100 to get load in units of xx.xx % */
    uint32_t hwi_load; /**< divide by 100 to get load in units of xx.xx % */
    uint32_t swi_load; /**< divide by 100 to get load in units of xx.xx % */

} app_perf_stats_cpu_load_t;


/**
 * \brief Performance point, all time in units of usecs
 */
typedef struct {

    char     name[APP_PERF_POINT_NAME_MAX];
    uint64_t tmp;
    uint64_t sum;
    uint64_t avg;
    uint64_t min;
    uint64_t max;
    uint64_t num;

} app_perf_point_t;

/**
 * \brief CPU task statistics information
 */
typedef struct {

    char task_name[APP_PERF_STATS_TASK_NAME_MAX]; /**< task name */
    uint32_t task_load; /**< divide by 100 to get load in units of xx.xx % */

} app_perf_stats_cpu_task_stats_t;

/**
 * \brief Detailed CPU Task Stats information
 */
typedef struct {

    uint32_t num_tasks;     /**< Number of tasks in task_stats[] */
    uint32_t starting_task; /**< Starting task index (Requestor sets this value, index 0 of task_stats[] will correspond
                                 to the overall index of the starting task) */

    app_perf_stats_cpu_task_stats_t task_stats[APP_PERF_STATS_TASK_MAX]; /**< task level performance info */

} app_perf_stats_task_stats_t;

/**
 * \brief Detailed CPU Mem Stats information
 */
typedef struct {

    uint32_t num_tasks; /**< Number of tasks in task_stats[] */

    app_mem_stats_t mem_stats[APP_MEM_HEAP_MAX]; /**< heap information */

} app_perf_stats_mem_stats_t;

/**
 * \brief HWA load information
 */
typedef struct {

    uint64_t total_time; /**< Total time elasped, in units of usecs */
    uint64_t active_time; /**< Time the HWA was active, in units of usecs */
    uint64_t pixels_processed; /**< Number of pixels processed in active time */
    uint64_t last_timestamp; /**< Last time HWA was active, USED INTERNALLY, NOT TO BE USED BY USER */

} app_perf_stats_hwa_load_t;

/**
 * \brief HWA load information for all HWAs
 *
 * note, this information is retrived from a specific CPU. If HWA does not
 *       reside on that CPU total_time, active_time, pixels_processed will be 0
 */
typedef struct {

    app_perf_stats_hwa_load_t hwa_stats[APP_PERF_HWA_MAX]; /**< HWA load information */

} app_perf_stats_hwa_stats_t;


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

} app_perf_stats_ddr_stats_t;

/**
 * \brief OS Resource usage information
 */
typedef struct {

    uint32_t semaphore_count;  /**< total semaphores currently being allocated */
    uint32_t mutex_count;      /**< total mutexes currently being allocated */
    uint32_t queue_count;      /**< total queues currently being allocated */
    uint32_t event_count;      /**< total events currently being allocated */
    uint32_t heap_count;       /**< total heaps currently being allocated */
    uint32_t mailbox_count;    /**< total mailboxes currently being allocated */
    uint32_t task_count;       /**< total tasks currently being allocated */
    uint32_t clock_count;      /**< total clocks currently being allocated */
    uint32_t hwi_count;        /**< total hwis currently being allocated */
    uint32_t timer_count;      /**< total timers currently being allocated */

    uint32_t semaphore_peak;   /**< peak semaphores allocated since boot */
    uint32_t mutex_peak;       /**< peak mutexes allocated since boot */
    uint32_t queue_peak;       /**< peak queues allocated since boot */
    uint32_t event_peak;       /**< peak events allocated since boot */
    uint32_t heap_peak;        /**< peak heaps allocated since boot */
    uint32_t mailbox_peak;     /**< peak mailboxes allocated since boot */
    uint32_t task_peak;        /**< peak tasks allocated since boot */
    uint32_t clock_peak;       /**< peak clocks allocated since boot */
    uint32_t hwi_peak;         /**< peak hwis allocated since boot */
    uint32_t timer_peak;       /**< peak timers allocated since boot */

} app_perf_stats_os_stats_t;

/**
 * \brief TIOVX resource usage information
 */
typedef struct {

    uint32_t targets_in_cpu;
    uint32_t max_targets_per_kernel;
    uint32_t target_kernel_instance_max;
    uint32_t target_kernel_max;

} app_perf_stats_tiovx_stats_t;

/**
 * \brief TIOVX registration function pointer
 */
typedef void (*tiovxStats_Fxn) (app_perf_stats_tiovx_stats_t* tiovx_stats);

/**
 * \brief TIOVX registration functions
 */
typedef struct {
    tiovxStats_Fxn get_tiovx_stats;
} app_perf_registration_t;

/**
 * \brief Initialize perf statistics collector module
 *
 *        RTOS only API
 *        MUST be called before any other API
 */
int32_t appPerfStatsInit();

/**
 * \brief Initialize perf statistics collector module with registration functions
 *
 *        RTOS only API
 *        MUST be called before any other API
 */
int32_t appPerfStatsInitRegister(app_perf_registration_t * reg_fxns);

/**
 * \brief Initialize perf statistics collector module
 *
 *        RTOS only API
 *        MUST be called after IPC init
 */
int32_t appPerfStatsRemoteServiceInit();

/**
 * \brief Reset CPU load statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 */
int32_t appPerfStatsCpuLoadReset(uint32_t app_cpu_id);

/**
 * \brief Get CPU peformance statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param cpu_load [out] CPU performance statistics
 */
int32_t appPerfStatsCpuLoadGet(uint32_t app_cpu_id, app_perf_stats_cpu_load_t *cpu_load);

/**
 * \brief Get CPU peformance statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param cpu_stats [out] CPU task performance statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuTaskStatsGet(uint32_t app_cpu_id, app_perf_stats_task_stats_t *cpu_stats);

/**
 * \brief Print CPU peformance statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param cpu_stats  [in] Detailed CPU task performance statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuTaskStatsPrint(uint32_t app_cpu_id, app_perf_stats_task_stats_t *cpu_stats);

/**
 * \brief Get CPU task stack statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param cpu_stats [out] CPU task stack statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuTaskStacksStatsGet(uint32_t app_cpu_id, app_perf_stats_task_stats_t *cpu_stats);

/**
 * \brief Print CPU task stack statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param cpu_stats  [in] Detailed CPU task stack statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuTaskStacksStatsPrint(uint32_t app_cpu_id, app_perf_stats_task_stats_t *cpu_stats);

/**
 * \brief Get Memory statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param cpu_stats [out] CPU mem performance statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuMemStatsGet(uint32_t app_cpu_id, app_perf_stats_mem_stats_t *cpu_stats);

/**
 * \brief Print CPU peformance statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param cpu_stats  [in] Detailed CPU mem performance statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuMemStatsPrint(uint32_t app_cpu_id, app_perf_stats_mem_stats_t *cpu_stats);

/**
 * \brief Print CPU load statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param cpu_load   [in] CPU load statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuLoadPrint(uint32_t app_cpu_id, app_perf_stats_cpu_load_t *cpu_load);

/**
 * \brief Print CPU load statistics for all enabled CPUs
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuLoadPrintAll();

/**
 * \brief Get CPU OS statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param os_stats [in] CPU os static memory statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuOsStatsGet(uint32_t app_cpu_id, app_perf_stats_os_stats_t *os_stats);

/**
 * \brief Get CPU TIOVX target statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param tiovx_stats [in] tiovx statistics information
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuTiovxStatsGet(uint32_t app_cpu_id, app_perf_stats_tiovx_stats_t *tiovx_stats);

/**
 * \brief Print CPU OS statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param os_stats  [out] Detailed CPU os memory statistics
 * \param showPeak  [in] flag to print peak numbers or not
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuOsStatsPrint(uint32_t app_cpu_id, app_perf_stats_os_stats_t *os_stats, uint32_t showPeak);

/**
 * \brief Print CPU TIOVX statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param tiovx_stats  [out] CPU TIOVX memory statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuTiovxStatsPrint(uint32_t app_cpu_id, app_perf_stats_tiovx_stats_t *tiovx_stats);

/**
 * \brief Print CPU peformance statistics for all enabled CPUs
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuStatsPrintAll();


/**
 * \brief Reset CPU load calc for all enabled CPUs
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
int32_t appPerfStatsCpuLoadResetAll();


/**
 * \brief Print all performance related information
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
int32_t appPerfStatsPrintAll();

/**
 * \brief Register a task for task load calculation
 *
 *        Linux, RTOS only API
 *        For linux, this API does nothing as of now
 *        For RTOS, task_handle MUST point to BIOS Task_Handle
 *
 * \return 0 on success
 */
int32_t appPerfStatsRegisterTask(void *task_handle, const char *name);

/**
 * \brief De-Initialize perf statistics collector module
 *
 *        RTOS only API
 *
 * \return 0 on success
 */
int32_t appPerfStatsDeInit();


/**
 * \brief Set name for a performance point
 *
 *        Also resets the performance point.
 *
 * \param prm [out] performance point
 * \param name [out] name of this profile point
 */
void appPerfPointSetName(app_perf_point_t *prm, const char *name);

/**
 * \brief Reset a performance point, MUST be called once before begin/end
 *
 * \param prm [out] performance point
 */
void appPerfPointReset(app_perf_point_t *prm);

/**
 * \brief Start a performance point
 *
 * \param prm [out] performance point
 */
void appPerfPointBegin(app_perf_point_t *prm);


/**
 * \brief End a performance point
 *
 * \param prm [out] performance point
 */
void appPerfPointEnd(app_perf_point_t *prm);


/**
 * \brief Print a performance point
 *
 * \param prm [in] performance point
 */
void appPerfPointPrint(app_perf_point_t *prm);


/**
 * \brief Print a performance point in units of FPS
 *
 * \param prm [in] performance point
 */
void appPerfPointPrintFPS(app_perf_point_t *prm);

/**
 * \brief Opens a .md file and returns the pointer to application
 *
 * \param output_file_path   [in] path to output file
 * \param output_file_prefix [in] name of output file (function will append .md)
 */
FILE *appPerfStatsExportOpenFile(const char *output_file_path, const char *output_file_prefix);

/**
 * \brief Exports the performance to a .md file
 *
 * \param fp                 [in] file pointer to .md performance file
 * \param perf_points        [in] array of performance points
 * \param num_points         [in] number of points in array
 *
 * \return 0 on success
 */
int32_t appPerfStatsExportAll(FILE *fp, app_perf_point_t *perf_points[], uint32_t num_points);

/**
 * \brief Closes performance file opened by appPerfStatsExportOpenFile
 *
 * \param fp                 [in] file pointer to .md performance file
 *
 * \return 0 on success
 */
int32_t appPerfStatsExportCloseFile(FILE *fp);

/**
 * \brief Update load numbers against a HWA
 *
 * \param id                   [in] HWA ID
 * \param active_time_in_usecs [in] Time the HWA was active
 * \param pixels_processed     [in] Number of pixels processed in this duration
 */
void appPerfStatsHwaUpdateLoad(app_perf_hwa_id_t id, uint32_t active_time_in_usecs, uint32_t pixels_processed);

/**
 * \brief Get HWA peformance statistics
 *
 *        Linux, RTOS API
 *
 * \param app_cpu_id [in] CPU ID
 * \param hwa_stats [out] HWA performance statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsHwaStatsGet(uint32_t app_cpu_id, app_perf_stats_hwa_stats_t *hwa_stats);

/**
 * \brief Print HWA load statistics
 *
 *        Linux, RTOS API
 *
 * \param hwa_load   [in] HWA load statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsHwaLoadPrint(app_perf_stats_hwa_stats_t *hwa_load);

/**
 * \brief Print HWA load statistics for all enabled CPUs
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
int32_t appPerfStatsHwaLoadPrintAll();

/**
 * \brief Reset HWA load calc for all HWAs
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
int32_t appPerfStatsHwaLoadResetAll();


/**
 * \brief Get HWA name
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
char *appPerfStatsGetHwaName(app_perf_hwa_id_t hwa_id);


/**
 * \brief Get DDR BW statistics
 *
 *        Linux, RTOS API
 *
 * \param ddr_stats [out] DDR performance statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsDdrStatsGet(app_perf_stats_ddr_stats_t *ddr_stats);

/**
 * \brief Print DDR BW statistics
 *
 *        Linux, RTOS API
 *
 * \param ddr_load [out] DDR performance statistics
 *
 * \return 0 on success
 */
int32_t appPerfStatsDdrStatsPrint(app_perf_stats_ddr_stats_t *ddr_load);

/**
 * \brief Print DDR BW statistics
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
int32_t appPerfStatsDdrStatsPrintAll();

/**
 * \brief Reset DDR BW statistics
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
int32_t appPerfStatsDdrStatsResetAll();

/**
 * \brief Reset all perf stats
 *
 *        Linux, RTOS API
 *
 * \return 0 on success
 */
void appPerfStatsResetAll();

/* @} */

#ifdef __cplusplus
}
#endif

#endif
