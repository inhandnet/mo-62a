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

#ifndef APP_CLI_H
#define APP_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_CLI_MAX_PROMPT_NAME     (16u)

/* command handler callback */
typedef int32_t (*app_cli_cmd_handler_f)(int argc, char *argv[]);

/* callback to write string to console device */
typedef int (*app_cli_device_send_string_f)(char *string, uint32_t max_size);

/* callback to read string from console device */
typedef int (*app_cli_device_get_string_f)(char *string, uint32_t max_size, uint32_t *string_size);

/* CLI init parameters */
typedef struct {
    app_cli_device_send_string_f device_write; /* device specific callback to write string to device, by default appUartWriteString() is used */
    app_cli_device_get_string_f device_read;  /* device specific callback to read string from device, by default appUartReadString() is used */
    char cli_prompt_name[APP_CLI_MAX_PROMPT_NAME];   /* CLI prompt name, by default "ti" will be used */
} app_cli_init_prm_t;

/* set default parameters for CLI */
void appCliInitPrmSetDefault(app_cli_init_prm_t *);

/* init CLI */
int32_t appCliInit(app_cli_init_prm_t *);

/* de-init CLI */
int32_t appCliDeInit();

/* Register a handler for system command 'cmd'
 * A system command is a command that can be invoked at point during application invokation
 * ex, while a application use-case is running
 */
int32_t appCliRegisterSystemCmd(char *cmd, char *desc, app_cli_cmd_handler_f cmd_handler);

/* Register a handler for application command 'cmd'
 * Once a application command is invoke, additional application commands
 * cannot be invoked until this application finishes execution.
 * System commands and "Application sub-commands" can be invoked while
 * a application is running.
 * An application handler should register application specific sub-commands
 * which can be invoked while a application runs
 */
int32_t appCliRegisterAppCmd(char *cmd, char *desc, app_cli_cmd_handler_f cmd_handler);

/* Register a handler for application sub-commands 'cmd'
 * This are commands specific to the currently running application
 * ex, handler to stop current application would be different for different
 * applications.
 *
 * This commands can be invoked only after a application command is executed
 */
int32_t appCliRegisterAppSubCmd(char *cmd, char *desc, app_cli_cmd_handler_f cmd_handler);

/* waits to read input from user and invokes handler based on
   the command that is entered.
   After handling one command, the function returns.
   If unsupport command is entered, function returns.
   User should call this function in a loop to get the effect of a CLI.
   If user enters 'exit', is_exit flag is set to 1
   User can use this flag to break out of the loop.
 */
int32_t appCliShowPrompt(uint32_t *is_exit);

/* Show Cli Bannner */
void appCliShowBanner(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_CLI_H */

