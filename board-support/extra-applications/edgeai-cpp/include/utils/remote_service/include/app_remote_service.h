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

#ifndef __APP_REMOTE_SERVICE_H__
#define __APP_REMOTE_SERVICE_H__

#include <stdint.h>

/**
 * \defgroup group_vision_apps_utils_remote_service Remote Service APIs (RTOS only)
 *
 * \brief These APIs allows user to invoke functions or services on remote cores by abstracting the
 *        low level IPC details.
 *
 *        These APIs are meant to be used for control like functions and not to
 *        execute real-time algorithms.
 *
 *        Also service commands are executed in a single thread so multiple requests
 *        to the same CPU get serialized. Also once a service commands executes it runs to
 *        completion and only then moves on to another service commands.
 *
 *        The focus of these APIs is simplicity rather than low latency, efficiency.
 *
 * \ingroup group_vision_apps_utils
 *
 * @{
 */

/**
 * \brief Flag to indicate that this service command does not need a ACK
 */
#define APP_REMOTE_SERVICE_FLAG_NO_WAIT_ACK         (0x00000001u)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief Remote service init params
 */
typedef struct
{
    uint32_t rpmsg_rx_endpt; /**< rpmsg port to use to receive requests */

} app_remote_service_init_prms_t;

/**
 * \brief Remote service handler
 *
 * \param service_name [in] service name for which this handler is invoked.
 * \param cmd [in] service command
 * \param prm [in/out] service command parameters
 * \param prm_size [in] service command parameter size
 * \param flags [in] service command flags
 *
 * \return 0 on success, else failure
 */
typedef int32_t (*app_remote_service_handler_t)(char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags);

/**
 * \brief Init prm with default parameters.
 *
 * Users are recommended to call this API before calling appRemoteServiceInit
 *
 * \param prm [in] Initialization parameters
 */
void appRemoteServiceInitSetDefault(app_remote_service_init_prms_t *prm);

/**
 * \brief Init remote service task
 *
 * \param prm [in] Initialization parameters
 *
 * \return 0 on success, else failure
 */
int32_t appRemoteServiceInit(app_remote_service_init_prms_t *prm);

/**
 * \brief Remote service register
 *
 * \param service_name [in] service name
 * \param handler [in] handler to invoke when a command is received for 'service_name' service
 *
 * \return 0 on success, else failure
 */
int32_t appRemoteServiceRegister(const char *service_name, app_remote_service_handler_t handler);

/**
 * \brief Remote service unregister
 *
 * \param service_name [in] service name
 *
 * \return 0 on success, else failure
 */
int32_t appRemoteServiceUnRegister(const char *service_name);

/**
 * \brief Run a remote service
 *
 * \param dst_app_cpu_id [in] CPU on which to execute the service command
 * \param service_name [in] Service handler to execute
 * \param cmd [in] service command to pass to the handler
 * \param prm [in] service command parameter to pass to the handler
 * \param prm_size [in] service command parameter size in bytes
 * \param flags [in] Service execution flags
 *
 * \return 0 on success, else failure
 */
int32_t appRemoteServiceRun(uint32_t dst_app_cpu_id, const char *service_name, uint32_t cmd, void *prm, uint32_t prm_size, uint32_t flags);

/**
 * \brief DeInit remote service task
 *
 * \return 0 on success, else failure
 */
int32_t appRemoteServiceDeInit();

#ifdef __cplusplus
}
#endif

/* @} */

#endif
