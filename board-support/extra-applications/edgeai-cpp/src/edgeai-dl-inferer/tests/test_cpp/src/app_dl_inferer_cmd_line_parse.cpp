/*
 *  Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Standard headers. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <filesystem>

/* Module headers. */
#include <test_cpp/include/app_dl_inferer_cmd_line_parse.h>

namespace ti::app_dl_inferer::common
{
using namespace ti::dl_inferer::utils;

static void showUsage(const char *name)
{
    printf("# \n");
    printf("# %s PARAMETERS [OPTIONAL PARAMETERS]\n", name);
    printf("# PARAMETERS:\n");
    printf("#  [--model_directory  |-d Path to Model Directory.]\n");
    printf("#  [--test_image  |-i Path to test image.]\n");
    printf("# OPTIONAL PARAMETERS:\n");
    printf("#  [--viz_threshold  |-v Threshold for Object Detection / Human Pose Estimation. Default is 0.6.]\n");
    printf("#  [--alpha  |-a Alpha value for blending in semantic segmentation. Default is 0.4.\n");
    printf("#  [--top_n   |-t Top N classes for image classification. Default is 5.]\n");
    printf("#  [--disable_tidl    |-e Disable TIDL.]\n");
    printf("#  [--log-level  |-l Logging level to enable. [0: DEBUG 1:INFO 2:WARN 3:ERROR]. Default is 2.\n");
    printf("#  [--help       |-h]\n");
    printf("# \n");
    printf("# (C) Texas Instruments 2022\n");
    printf("# \n");
    exit(0);
}

void
CmdlineArgs::parse(int32_t        argc,
                   char          *argv[])
{
    int32_t longIndex;
    int32_t opt;
    static struct option long_options[] = {
        {"help",            no_argument,       0, 'h'},
        {"disable_tidl",    no_argument,       0, 'e' },
        {"top_n",           no_argument,       0, 't' },
        {"alpha",           no_argument,       0, 'a' },
        {"viz_threshold",   no_argument,       0, 'v' },
        {"test_image",      required_argument, 0, 'i' },
        {"model_directory", required_argument, 0, 'd' },
        {0,                 0,                 0,  0  }
    };

    while ((opt = getopt_long(argc, argv,"-het:a:v:i:d:",
                   long_options, &longIndex )) != -1)
    {
        switch (opt)
        {

            case 'd' :
                modelDirectory = optarg;
                break;

            case 'i' :
                testImage = optarg;
                break;

            case 'v' :
                vizThreshold = std::stof(optarg);
                break;

            case 'a' :
                alpha = std::stof(optarg);
                break;

            case 't' :
                topN = std::stoi(optarg);
                break;

            case 'e' :
                enableTidl = false;
                break;
            
            case 'l' :
                logLevel = optarg;
                break;
            
            case 'h' :
            default:
                showUsage(argv[0]);
                exit(-1);
        }
    }

    if (!std::filesystem::exists(modelDirectory))
    {
        DL_INFER_LOG_ERROR("The directory [%s] does not exist.\n",
                           modelDirectory.c_str());
        exit(-1);
    }
    if (!std::filesystem::exists(testImage))
    {
        DL_INFER_LOG_ERROR("The file [%s] does not exist.\n",
                            testImage.c_str());
        exit(-1);
    }

    return;

}

} // namespace  ti::app_dl_inferer::common