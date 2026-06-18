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

#ifndef APP_RTOS_H
#define APP_RTOS_H

#include <stdint.h>

/**
 * \defgroup group_vision_apps_utils_rtos Application RTOS OSAL/DPL APIs
 *
 * \brief This section contains APIs for providing abstraction between PDK OSAL and MCU+SDK DPL.
 *
 * \ingroup group_vision_apps_utils
 *
 * \{
 */
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 *  \anchor app_rtos_status_e
 *  \name App Rtos status code
 *  @{
 */
/*!
 *  @brief    Status codes for App Rtos APIs
 */
typedef int32_t app_rtos_status_t;
#define APP_RTOS_STATUS_SUCCESS                           (0)
/*! API completed successfully */
#define APP_RTOS_STATUS_FAILURE                      (-(int32_t)1)
/*! API failed */
#define APP_RTOS_STATUS_TIMEOUT                      (-(int32_t)2)
/*! API failed because of a timeout */
/* @} */

/**
 *  \anchor app_rtos_semaphore_mode_e
 *  \name Semaphore Modes
 *  @{
 */
/*!
 *  @brief    Mode of the semaphore
 */
typedef uint32_t app_rtos_semaphore_mode_t;
#define APP_RTOS_SEMAPHORE_MODE_COUNTING                (0x0U)
#define APP_RTOS_SEMAPHORE_MODE_BINARY                  (0x1U)
#define APP_RTOS_SEMAPHORE_MODE_MUTEX                   (0x2U)
/* @} */

/*!
 *  @brief    Wait forever define
 */
#define APP_RTOS_SEMAPHORE_WAIT_FOREVER (~((uint32_t)0U))

/*!
 *  @brief    No wait define
 */
#define APP_RTOS_SEMAPHORE_NO_WAIT       ((uint32_t)0U)


/**
 * \brief Semaphore parameters
 *
 */
typedef struct {

    app_rtos_semaphore_mode_t mode;  /**< Semaphore Mode */
    uint32_t initValue;              /**< Initial value for the semaphore */
    uint32_t maxValue;               /**< Maximum value of counting semaphore. Not used for binary semaphore */
} app_rtos_semaphore_params_t;

/**
 * \brief Task parameters
 *
 */
typedef struct {

    const char   *name;         /**< Pointer to task name */
    uint32_t      stacksize;    /**< Size of stack in units of bytes */
    uint8_t      *stack;        /**< Pointer to stack memory, MUST be aligned based on CPU architecture, typically atleast 32b on 32b systems */
    uint32_t      priority;     /**< Task priority */
    void         *arg0;         /**< User arguments that are passed back as parameter to task main */
    void         *arg1;         /**< User arguments that are passed back as parameter to task main */
    void         (*taskfxn)(void *arg0, void *arg1);      /**< Entry point function to the task */
    void         *userData;     /**< User-defined data */
} app_rtos_task_params_t;

/*!
 *  \brief    Opaque client reference to an instance of a semaphore
 */
typedef  void *app_rtos_semaphore_handle_t;

/*!
 *  \brief    Opaque client reference to an instance of a task
 */
typedef  void *app_rtos_task_handle_t;

/*!
 *  \brief  Initialize params structure to default values.
 *
 *  The default parameters are:
 *   - mode:      APP_RTOS_SEMAPHORE_MODE_COUNTING
 *   - maxValue:  255
 *   - initValue: 0
 *
 *  \param params  Pointer to the instance configuration parameters.
 */
void appRtosSemaphoreParamsInit(app_rtos_semaphore_params_t *params);

/**
 * \brief Creates a semaphore instance
 *
 * \param params [in] parameters for semaphore creation
 *
 * \return app_rtos_semaphore_handle_t on success or a NULL on an error
 */
app_rtos_semaphore_handle_t appRtosSemaphoreCreate(app_rtos_semaphore_params_t params);

/*!
 *  \brief  Function to delete a semaphore.
 *
 *  \param  semhandle  A app_rtos_semaphore_handle_t returned from ::appRtosSemaphoreCreate
 *
 *  \return Status of the functions
 *    - APP_RTOS_STATUS_SUCCESS: Deleted the semaphore instance
 *    - APP_RTOS_STATUS_FAILURE: Failed to delete the semaphore instance
 */
app_rtos_status_t appRtosSemaphoreDelete(app_rtos_semaphore_handle_t *semhandle);

/*!
 *  \brief  Function to pend (wait) on a semaphore.
 *
 *  \param  semhandle  A app_rtos_semaphore_handle_t returned from ::appRtosSemaphoreCreate
 *
 *  \param  timeout Timeout (in milliseconds) to wait for the semaphore to
 *                  be posted (signaled).
 *
 *  \return Status of the functions
 *    - APP_RTOS_STATUS_SUCCESS: Obtain the semaphore
 *    - APP_RTOS_STATUS_TIMEOUT: Timed out. Semaphore was not obtained.
 *    - APP_RTOS_STATUS_FAILURE: Non-time out failure.
 */
app_rtos_status_t appRtosSemaphorePend(app_rtos_semaphore_handle_t semhandle,
                                         uint32_t timeout);

/*!
 *  \brief  Function to post (signal) a semaphore.
 *
 *  \param  semhandle  A app_rtos_semaphore_handle_t returned from ::appRtosSemaphoreCreate
 *
 *  \return Status of the functions
 *    - APP_RTOS_STATUS_SUCCESS: Released the semaphore
 *    - APP_RTOS_STATUS_FAILURE: Failed to post the semaphore
 */
app_rtos_status_t appRtosSemaphorePost(app_rtos_semaphore_handle_t semhandle);

/*!
 *  \brief  Function to clear a semaphore for reuse.
 *
 *  \param  semhandle  A app_rtos_semaphore_handle_t returned from ::appRtosSemaphoreCreate
 *
 *  \return Status of the functions
 *    - APP_RTOS_STATUS_SUCCESS: Reset the semaphore
 *    - APP_RTOS_STATUS_FAILURE: Failed to reset the semaphore
 */
app_rtos_status_t appRtosSemaphoreReset(app_rtos_semaphore_handle_t semhandle);


/*!
 *  \brief  Initialize params structure to default values.
 *
 *  \param params  Pointer to the instance configuration parameters.
 */
void appRtosTaskParamsInit(app_rtos_task_params_t *params);

/*!
 *  \brief  Function to create a task.
 *
 *  \param  params  Pointer to the instance configuration parameters.
 *
 *  \return app_rtos_task_handle_t on success or a NULL on an error
 */
app_rtos_task_handle_t appRtosTaskCreate(const app_rtos_task_params_t *params);

/*!
 *  \brief  Function to delete a task.
 *
 *  \param  handle  A app_rtos_semaphore_handle_t returned from ::appRtosSemaphoreCreate
 *
 *  \return Status of the functions
 *    - APP_RTOS_STATUS_SUCCESS: Deleted the task
 *    - APP_RTOS_STATUS_FAILURE: Failed to delete the task
 */
app_rtos_status_t appRtosTaskDelete(app_rtos_task_handle_t *handle);

/*!
 *  \brief  Check if task is terminated.
 *
 * Typically a task MUST be terminated before it can be deleted.
 *
 *  \return 0: task is not terminated, 1: task is terminated
 */
uint32_t appRtosTaskIsTerminated(app_rtos_task_handle_t handle);

/*!
 *  \brief  Function for Task yield
 */
void appRtosTaskYield(void);

/*!
 *  \brief  Function for Task sleep in units of OS tick
 *
 *  \param timeout  sleep ticks.
 */
void appRtosTaskSleep(uint32_t timeout);


/*!
 *  \brief  Function for Task sleep in units of msecs
 *
 *  \param timeoutInMsecs  sleep in units of msecs.
 */
void appRtosTaskSleepInMsecs(uint32_t timeoutInMsecs);


#ifdef __cplusplus
}
#endif /* __cplusplus */

/* \} */

#endif

