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

#ifndef APP_IPC_LINUX_PRIV_H_
#define APP_IPC_LINUX_PRIV_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <pthread.h>
#include <ti_rpmsg_char.h>

#include <utils/console_io/include/app_log.h>
#include <utils/console_io/src/app_log_priv.h>
#include <utils/ipc/include/app_ipc.h>

/* #define APP_IPC_DEBUG */

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     (_a > _b) ? _a : _b; })

typedef struct {

    app_ipc_init_prm_t prm;
    void *hw_spin_lock_addr;
    app_ipc_notify_handler_f ipc_notify_handler;
    int tx_fds[APP_IPC_CPU_MAX];
    uint32_t local_endpt[APP_IPC_CPU_MAX];
    rpmsg_char_dev_t *rcdev[APP_IPC_CPU_MAX];
    pthread_t task;
    int unblockfd;

} app_ipc_obj_t;


extern app_ipc_obj_t g_app_ipc_obj;


int32_t appIpcHwLockInit(void);
int32_t appIpcCreateRpmsgRxTask(app_ipc_obj_t *obj);
int32_t appIpcDeleteRpmsgRxTask(app_ipc_obj_t *obj);

/**
 * \brief Create IPC TX channel using rpmsg
 *
 * ONLY VALID on Linux
 *
 */
int32_t appIpcCreateTxCh(uint32_t remote_app_cpu_id, uint32_t remote_endpt, uint32_t *local_endpt, rpmsg_char_dev_t **rcdev, char *eptdev_name);

/**
 * \brief Delete IPC RX/TX channel using rpmsg
 *
 * ONLY VALID on Linux
 *
 */
int32_t appIpcDeleteCh(rpmsg_char_dev_t *rcdev);

#endif /* APP_IPC_LINUX_PRIV_H_ */
