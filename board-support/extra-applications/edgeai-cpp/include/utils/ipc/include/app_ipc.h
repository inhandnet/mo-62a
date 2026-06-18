/*
 *
 * Copyright (c) 2017 Texas Instruments Incorporated
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

#ifndef APP_IPC_H_
#define APP_IPC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* #define APP_IPC_DEBUG */

/**
 * \defgroup group_vision_apps_utils_ipc Inter-processor communication (IPC) APIs
 *
 * \brief This section contains APIs for Inter-processor communication (IPC)
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

/**
 *  \anchor app_ipc_cpu
 *  \name Application defined CPU ID's
 *
 *  @{
 */

#if defined (SOC_J721E)
/** \brief CPU ID */
#define APP_IPC_CPU_MPU1_0  ( 0u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_0  ( 1u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_1  ( 2u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU2_0  ( 3u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU2_1  ( 4u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU3_0  ( 5u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU3_1  ( 6u)
/** \brief CPU ID */
#define APP_IPC_CPU_C6x_1   ( 7u)
/** \brief CPU ID */
#define APP_IPC_CPU_C6x_2   ( 8u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_1   ( 9u)
/** \brief Max CPU ID */
#define APP_IPC_CPU_MAX     (10u)
/** \brief Invalid CPU ID */
#define APP_IPC_CPU_INVALID (0xFFu)
#endif

#if defined (SOC_J721S2)
/** \brief CPU ID */
#define APP_IPC_CPU_MPU1_0  ( 0u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_0  ( 1u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_1  ( 2u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU2_0  ( 3u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU2_1  ( 4u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU3_0  ( 5u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU3_1  ( 6u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_1   ( 7u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_2   ( 8u)
/** \brief Max CPU ID */
#define APP_IPC_CPU_MAX     ( 9u)
/** \brief Invalid CPU ID */
#define APP_IPC_CPU_INVALID (0xFFu)
#endif

#if defined (SOC_J784S4)
/** \brief CPU ID */
#define APP_IPC_CPU_MPU1_0  ( 0u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_0  ( 1u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_1  ( 2u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU2_0  ( 3u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU2_1  ( 4u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU3_0  ( 5u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU3_1  ( 6u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU4_0  ( 7u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU4_1  ( 8u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_1   ( 9u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_2   (10u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_3   (11u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_4   (12u)
/** \brief Max CPU ID */
#define APP_IPC_CPU_MAX     (13u)
/** \brief Invalid CPU ID */
#define APP_IPC_CPU_INVALID (0xFFu)
#endif

#if defined (SOC_J742S2)
/** \brief CPU ID */
#define APP_IPC_CPU_MPU1_0  ( 0u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_0  ( 1u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_1  ( 2u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU2_0  ( 3u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU2_1  ( 4u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU3_0  ( 5u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU3_1  ( 6u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU4_0  ( 7u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU4_1  ( 8u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_1   ( 9u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_2   (10u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_3   (11u)
/** \brief CPU ID */
#define APP_IPC_CPU_MAX     (12u)
/** \brief Invalid CPU ID */
#define APP_IPC_CPU_INVALID (0xFFu)
#endif

#if defined (SOC_AM62A)
/** \brief CPU ID */
#define APP_IPC_CPU_MPU1_0  ( 0u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_0  ( 1u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_1   ( 2u)
/** \brief Max CPU ID */
#define APP_IPC_CPU_MAX     ( 3u)
/** \brief Invalid CPU ID */
#define APP_IPC_CPU_INVALID (0xFFu)
#if defined(MCU_PLUS_SDK)
#define    IPC_MAX_PROCS        (3U)    /**< Maximum Processors */
#endif
#endif

#if defined (SOC_J722S)
/** \brief CPU ID */
#define APP_IPC_CPU_MPU1_0  ( 0u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU1_0  ( 1u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU2_0  ( 2u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_1   ( 3u)
/** \brief CPU ID */
#define APP_IPC_CPU_C7x_2   ( 4u)
/** \brief CPU ID */
#define APP_IPC_CPU_MCU_R5F ( 5u)
/** \brief Max CPU ID */
#define APP_IPC_CPU_MAX     ( 6u)
/** \brief Invalid CPU ID */
#define APP_IPC_CPU_INVALID (0xFFu)
#if defined(MCU_PLUS_SDK)
#define    IPC_MAX_PROCS        (6U)    /**< Maximum Processors */
#endif
#endif

/* @} */

/** \brief Max lock ID for HW locks */
#define APP_IPC_HW_LOCK_MAX     (256u)

/** \brief Timeout value to use to wait forever */
#define APP_IPC_WAIT_FOREVER    (0xFFFFFFFFu)

/**
 *  \anchor app_ipc_port_id
 *  \name RPMessage end points used by various services
 *
 *  @{
 */

/** \brief RPMsg Port used for TIOVX IPC */
#define APP_IPC_TIOVX_RPMSG_PORT_ID      (13u)

#if defined (SOC_AM62A)
/** \brief RPMsg Port used for Remote service */
#define APP_IPC_REMOTE_SERVICE_RPMSG_PORT_ID      (14u)

/** \brief RPMsg Port used for echo test */
#define APP_IPC_ECHO_TEST_RPMSG_PORT_ID  (21u)
#else
/** \brief RPMsg Port used for Remote service */
#define APP_IPC_REMOTE_SERVICE_RPMSG_PORT_ID      (21u)

/** \brief RPMsg Port used for echo test */
#define APP_IPC_ECHO_TEST_RPMSG_PORT_ID  (14u)
#endif

/** \brief RPMsg Port used for RPMsg proto echo test */
#define APP_IPC_RPMSG_PROTO_ECHO_TEST_RPMSG_PORT_ID  (12u)

/* @} */

/**
 * \brief Callback that is invoke when current CPU receives a IPC notify
 *
 * \param src_cpu_id [in] source CPU which generated this callback, see APP_IPC_CPU_*
 * \param payload  [in] payload or message received from source CPU to current CPU
 *
 */
typedef void (*app_ipc_notify_handler_f)(uint32_t src_cpu_id, uint32_t payload);


/**
 * \brief IPC initialization parameters
 */
typedef struct {

    uint32_t num_cpus; /**< Number of CPUs enabled for IPC */
    uint32_t enabled_cpu_id_list[APP_IPC_CPU_MAX]; /**< List of CPU ID enabled for IPC, see APP_IPC_CPU_* */
    uint32_t tiovx_rpmsg_port_id; /**< RPMsg port ID to use for TIOVX messages */
    void     *tiovx_obj_desc_mem;  /**< Pointer to TIOVX obj desc shared memory */
    uint32_t tiovx_obj_desc_mem_size; /**< Size of TIOVX obj desc shared memory */
    void     *ipc_vring_mem; /**< Pointer to IPC vring memory */
    uint32_t ipc_vring_mem_size; /**< Size of IPC vring memory */
    void     *tiovx_log_rt_mem;  /**< Pointer to TIOVX shared memory for run-time logging */
    uint32_t tiovx_log_rt_mem_size; /**< Size of TIOVX shared memory for run-time logging */
    uint32_t self_cpu_id;  /**< Self proc ID */
    void     *ipc_resource_tbl; /**< Pointer to IPC Resource Table */
    uint32_t enable_tiovx_ipc_announce; /**< Flag to set ipc rpmsg announce from utils/apps */
} app_ipc_init_prm_t;

/**
 * \brief Set IPC init parameters to default state
 *
 * Recommend to call this API before calling appIpcInit.
 *
 * \param prm [out] Parameters set to default
 */
void appIpcInitPrmSetDefault(app_ipc_init_prm_t *prm);

/**
 * \brief Initialize IPC module
 *
 * \param prm [in] Initialization parameters
 *
 * This API is called internally within each respective core's appInit
 * function.  It should not be called directly.
 *
 * \return 0 on success, else failure
 */
int32_t appIpcInit(app_ipc_init_prm_t *prm);

/**
 * \brief De-Initialize IPC module
 *
 * This API is called internally within each respective core's appDeinit
 * function.  It should not be called directly.
 *
 * \return 0 on success, else failure
 */
int32_t appIpcDeInit(void);

/**
 * \brief Register callback to invoke on receiving a notify message
 *
 * \param handler [in] Notify handler
 *
 * \return 0 on success, else failure
 */
int32_t appIpcRegisterNotifyHandler(app_ipc_notify_handler_f handler);

/**
 * \brief Send a notify message to a given CPU
 *
 * \param dest_cpu_id [in] Destinatin CPU ID, see APP_IPC_CPU_*
 * \param payload [in] payload to send as part of notify
 *
 * \return 0 on success, else failure
 */
int32_t appIpcSendNotify(uint32_t dest_cpu_id, uint32_t payload);

/**
 * \brief Send a notify message to a given CPU
 *
 * \param dest_cpu_id [in] Destinatin CPU ID, see APP_IPC_CPU_*
 * \param payload [in] payload to send as part of notify
 * \param port_id [in] port used for IPC
 *
 * \return 0 on success, else failure
 */
int32_t appIpcSendNotifyPort(uint32_t dest_cpu_id, uint32_t payload, uint32_t port_id);

/**
 * \brief Get current CPU ID
 *
 * \return current CPU ID, see APP_IPC_CPU_*
 */
uint32_t appIpcGetSelfCpuId(void);

/**
 * \brief Get current CPU Port ID
 *
 * \return current CPU Port ID
 */
uint32_t appIpcGetHostPortId(uint16_t cpu_id);

/**
 * \brief Check if a CPU is enabled in current system for IPC
 *
 * \param cpu_id [in] CPU ID, see APP_IPC_CPU_*
 *
 * \return 1 if CPU is enabled, 0 if CPU is disabled
 */
uint32_t appIpcIsCpuEnabled(uint32_t cpu_id);

/**
 * \brief Acquire a system wide HW lock
 *
 * This API will spin until the lock is acquired or timeout is hit.
 * It is recoemended to take a CPU level lock like a semaphore or mutex before
 * calling this API.
 *
 * \param lock_id [in] HW lock ID to use
 * \param timeout [in] Timeout in units of usecs.
 *
 *
 * \return 0 if lock is acquire, -2 if timeout is hit, else failure
 */
int32_t appIpcHwLockAcquire(uint32_t lock_id, uint32_t timeout);

/**
 * \brief Release a system wide HW lock
 *
 * \param lock_id [in] HW lock ID to use
 */
int32_t appIpcHwLockRelease(uint32_t lock_id);

/**
 * \brief Get base address and size of memory region assigned for TIOVX obj_desc's
 *
 * This is used by TIOVX as shared region and is typically non-cached at all CPUs.
 * It can be cache at a CPU if the CPU support cache coherency.
 *
 * \param addr [out] Base address of shared region
 * \param size [out] Size of shared region
 */
int32_t appIpcGetTiovxObjDescSharedMemInfo(void **addr, uint32_t *size);

/**
 * \brief Get base address and size of memory region assigned for TIOVX run-time logging
 *
 * This is used by TIOVX as shared region and is typically non-cached at all CPUs.
 * It can be cache at a CPU if the CPU support cache coherency.
 *
 * \param shm_base [out] Base address of shared region
 * \param shm_size [out] Size of shared region
 */
void appIpcGetTiovxLogRtSharedMemInfo(void **shm_base, uint32_t *shm_size);

/**
 * \brief Convert from APP_CPU_xxx to IPC LLD CPU ID
 *
 */
uint32_t appIpcGetIpcCpuId(uint32_t app_cpu_id);


/**
 * \brief Get APP_CPU_xxx from CPU name
 *
 */
uint32_t appIpcGetAppCpuId(char *name);

/**
 * \brief Get CPU name from CPU ID
 *
 */
const char *appIpcGetCpuName(uint32_t app_cpu_id);

/**
 * \brief Start echo test
 *
 */
int32_t appIpcEchoTestStart(void);

#if defined (SOC_AM62A)
/**
 * \brief API's called during remote core shutdown
 */
void appIpcSendShutdownAck(void);
uint32_t appIpcShutDownReceived(void);
#endif

#ifdef __cplusplus
}
#endif

/* @} */

#endif /* APP_IPC_H_ */

