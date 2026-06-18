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

#ifndef APP_LOG_H_
#define APP_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * \defgroup group_vision_apps_utils_log Logging APIs
 *
 * \brief This section contains APIs for logging from remote cores to a
 *        central host core which prints all the logs to a common console.
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

/** \brief Max CPUs that are participating in the logging */
#define APP_LOG_MAX_CPUS        (16u)

/** \brief CPU name to use as prefix while logging */
#define APP_LOG_MAX_CPU_NAME    (16u)

/** \brief Size of memory used for logging by one CPU */
#define APP_LOG_PER_CPU_MEM_SIZE   (16*1024u - 32u)

/** \brief Flag to check if log area is valid or not */
#define APP_LOG_AREA_VALID_FLAG (0x1357231u)

/** \brief Callback to write string to console device */
typedef void (*app_log_device_send_string_f)(char *string, uint32_t max_size);

/** \brief Shared memory structure used for logging by a specific CPU
 *
 *  This state of the log buffer, user can ignore it.
 */
typedef struct {

    volatile uint32_t log_rd_idx; /**< Init by reader to 0 */

    volatile uint32_t log_wr_idx; /**< Init by writer to 0 */

    volatile uint32_t log_area_is_valid; /**<  Init by writer to APP_LOG_AREA_VALID_FLAG.
                                      reader will ignore this CPU shared mem log
                                      until the writer sets this
                                      to APP_LOG_AREA_VALID_FLAG */

    volatile  uint32_t log_cpu_sync_state;    /**< CPU sync state */

    volatile  uint8_t  log_cpu_name[APP_LOG_MAX_CPU_NAME]; /**< Init by writer to CPU name, used by reader to add a prefix when writing to console device */



    volatile  uint8_t  log_mem[APP_LOG_PER_CPU_MEM_SIZE]; /**< memory into which logs are written by this CPU */
} app_log_cpu_shared_mem_t;

/**< \brief Shared memory structure for all CPUs, used by reader and writer CPUs
 *
 * User application MUST map this to the same physical address across all CPUs.
 * For non-coherent CPUs, this MUST map to a non-cache region
 * */
typedef struct {
    app_log_cpu_shared_mem_t cpu_shared_mem[APP_LOG_MAX_CPUS]; /**< CPU specific shared memory structure */
} app_log_shared_mem_t;

/** \brief Init parameters to use for appLogInit()
 *
 * */
typedef struct {
    app_log_shared_mem_t *shared_mem;       /**< Shared memory to use for logging, all CPUs must point to the same shared memory. This is physical address in case of linux */
    uint32_t self_cpu_index;                /**< Index into shared memory area for self CPU to use to use when writing. Two CPUs must not use the same CPU index */
    char     self_cpu_name[APP_LOG_MAX_CPU_NAME]; /**< self CPU name */
    uint32_t log_rd_task_pri;               /**< task priority for log reader, set to 0xFFFFFFFF for default task priority */
    uint32_t log_rd_poll_interval_in_msecs; /**< polling interval for log reader in msecs */
    uint32_t log_rd_max_cpus;               /**< Maximum CPUs that log into the shared memory */
    uint32_t log_rd_cpu_enable[APP_LOG_MAX_CPUS]; /**< 1: enable reading from log memory, else disable */
    app_log_device_send_string_f device_write; /**< Callback to write a string to a device specific function, by default this will be set to appUartWriteString() */
} app_log_init_prm_t;

/** \brief Initialize app_log_init_prm_t with default parameters
 *         always call this function before calling appLogInit
 *
 * It is recommended to call this API before calling appLogInit().
 * User should override init parameters after calling this API.
 *
 * \param prms [in] Init parameters
 */
void appLogInitPrmSetDefault(app_log_init_prm_t *prms);

/** \brief Init Log reader and log writer,
 *
 * \param prms [in] Init parameters.
 *
 * \return 0 on success, else failure.
 */
int32_t appLogRdInit(app_log_init_prm_t *prms);

/** \brief Init Log reader and log writer,
 *
 * \param prms [in] Init parameters.
 *
 * \return 0 on success, else failure.
 */
int32_t appLogWrInit(app_log_init_prm_t *prms);

/** \brief De-init log reader and log writer
 *
 * \return 0 on success, else failure.
 */
int32_t appLogRdDeInit();

/** \brief De-init log reader and log writer
 *
 * \return 0 on success, else failure.
 */
int32_t appLogWrDeInit();

/** \brief Write a string to shared memory
 *
 * \param format [in] string to log with variable number of arguments
 *
 * \return 0 on success, else failure.
 */
void appLogPrintf(const char *format, ...);

/** \brief Redirect printf to appLogPrintf
 *
 * \return 0 on success, else failure.
 */
int32_t appLogCioInit(void);

/** \brief De-init Redirect printf to appLogPrintf
 *
 */
void appLogCioDeInit(void);

/**
 * \brief Sync provided list of CPUs
 */
void appLogCpuSyncInit(uint32_t master_cpu_id, uint32_t self_cpu_id,
        uint32_t sync_cpu_id_list[], uint32_t num_cpus);


/* @} */

#ifdef __cplusplus
}
#endif

#endif

