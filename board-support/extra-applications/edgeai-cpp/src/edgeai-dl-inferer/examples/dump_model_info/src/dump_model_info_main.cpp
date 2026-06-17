/*
 *
 * Copyright (c) 2022 Texas Instruments Incorporated
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
/* Standard headers. */
#include <signal.h>
#include <getopt.h>

/* Module headers. */
#include <ti_dl_inferer.h>
#include <ti_dl_inferer_logger.h>

using namespace std;
using namespace ti::dl_inferer;
using namespace ti::dl_inferer::utils;

static void showUsage(const char *name)
{
    printf(" \n");
    printf("# \n");
    printf("# %s PARAMETERS [OPTIONAL PARAMETERS]\n", name);
    printf("# OPTIONS:\n");
    printf("#  --model       |-m Path to the model directory.\n");
    printf("#  [--log-level  |-l Logging level to enable. [0: DEBUG 1:INFO 2:WARN 3:ERROR]. Default is 2.\n");
    printf("#  [--help       |-h]\n");
    printf("# \n");
    printf("# \n");
    printf("# (c) Texas Instruments 2022\n");
    printf("# \n");
    printf("# \n");
    exit(0);
}

static void ParseCmdlineArgs(int32_t    argc,
                             char      *argv[],
                             string    &modelBasePath,
                             string    &configFile)
{
    int32_t longIndex;
    int32_t opt;
    static struct option long_options[] = {
        {"help",    no_argument,       0, 'h' },
        {"model",   required_argument, 0, 'm' },
        {0,         0,                 0,  0  }
    };
    LogLevel            logLevel{WARN};

    while ((opt = getopt_long(argc, argv,"hm:l:", 
                   long_options, &longIndex )) != -1)
    {
        switch (opt)
        {
            case 'm' :
                modelBasePath = optarg;
                break;

            case 'l' :
                logLevel = static_cast<LogLevel>(strtol(optarg, NULL, 0));
                break;

            case 'h' :
            default:
                showUsage(argv[0]);
                exit(-1);

        } // switch (opt)

    } // while ((opt = getopt_long(argc, argv

    // Validate the parameters
    if (modelBasePath.empty())
    {
        showUsage(argv[0]);
        exit(-1);
    }

    configFile = modelBasePath + "/param.yaml";

    logSetLevel(logLevel);

    return;

} // End of ParseCmdLineArgs()

int main(int argc, char * argv[])
{
    InfererConfig   infConfig;
    string          modelBasePath;
    string          configFile;
    int32_t         status;

    // Parse the command line options
    ParseCmdlineArgs(argc, argv, modelBasePath, configFile);

    // Populate infConfig
    status = infConfig.getConfig(modelBasePath, true, 1);

    if (status < 0)
    {
        printf("[%s:%d] ti::utils::getConfig() failed.\n",
               __FUNCTION__, __LINE__);
    }
    else
    {
        DLInferer  *inferer;

        inferer = DLInferer::makeInferer(infConfig);

        if (inferer == nullptr)
        {
            printf("[%s:%d] ti::DLInferer::makeInferer() failed.\n",
                   __FUNCTION__, __LINE__);
        }
        else
        {
            inferer->dumpInfo();
        }

        delete inferer;
    }

    return status;
}
