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
#include <stdlib.h>
#include <filesystem>

/* Third Party Headers. */
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

/* Module headers. */
#include <test_cpp/include/app_dl_inferer_cmd_line_parse.h>
#include <test_cpp/include/app_dl_inferer_utils.h>
#include <test_cpp/include/app_dl_inferer_inference.h>

/* DL Inferer. */
#include <ti_dl_inferer.h>
#include <ti_post_process.h>
#include <ti_pre_process_config.h>
#include <ti_dl_inferer_logger.h>

using namespace std;
using namespace ti::dl_inferer;
using namespace ti::dl_inferer::utils;
using namespace ti::post_process;
using namespace ti::pre_process;
using namespace ti::app_dl_inferer::common;

int main(int argc, char * argv[])
{

    vector<cv::Mat>         testImages;
    PostprocessImageConfig  postProcessConfig;
    InfererConfig           infConfig;
    string                  modelBasePath;
    int32_t                 status;
    LogLevel                logLevel{INFO};

    CmdlineArgs cmdArgs;
    /* Parse the command line options. */
    cmdArgs.parse(argc, argv);
    
    /* Later we can read from directory and push images to testImages vector. */
    /* Read Image. */
    cv::Mat testImage = cv::imread(cmdArgs.testImage);

    if (testImage.empty()) {
        DL_INFER_LOG_ERROR("[%s] is EMPTY.\n",cmdArgs.testImage.c_str());
        exit(-1);
    }

    testImages.push_back(testImage);

    // Set Log Level
    logLevel = static_cast<LogLevel>(strtol(cmdArgs.logLevel.c_str(), NULL, 0));
    logSetLevel(logLevel);

    // Populate infConfig
    status = infConfig.getConfig(cmdArgs.modelDirectory, cmdArgs.enableTidl, 1);

    if (status < 0)
    {
        DL_INFER_LOG_ERROR("[%s:%d] ti::utils::getConfig() failed.\n",
                            __FUNCTION__, __LINE__);
        exit(-1);
    }
    else
    {
        DLInferer  *inferer;

        inferer = DLInferer::makeInferer(infConfig);

        if (inferer == nullptr)
        {
            DL_INFER_LOG_ERROR("[%s:%d] ti::DLInferer::makeInferer() failed.\n",
                               __FUNCTION__, __LINE__);
            exit(-1);
        }

        const VecDlTensor  *dlInfOutputs;
        const VecDlTensor  *dlInfInputs;
        const DlTensor     *ifInpInfo;
        const DlTensor     *ifOutInfo;

        dlInfOutputs    = inferer->getOutputInfo();
        ifOutInfo       = &dlInfOutputs->at(0);
        dlInfInputs     = inferer->getInputInfo();
        ifInpInfo       = &dlInfInputs->at(0);

        // Populate postProcessConfig
        status = postProcessConfig.getConfig(cmdArgs.modelDirectory);
        if (status < 0)
        {
            DL_INFER_LOG_ERROR("[%s:%d] ti::utils::getConfig() failed.\n",
                                __FUNCTION__, __LINE__);
            exit(-1);
        }

        string modelName = postProcessConfig.modelName;
        printf("\n[MODEL] %s\n" , modelName.c_str());

        /* Run Inferer. */
        for (int32_t i = 0; i < testImages.size(); i++)
        {   
            /* Make Pre Proc Config fot this image. */
            PreprocessImageConfig       preProcCfg;
            preProcCfg.inDataWidth  = testImages[i].cols;
            preProcCfg.inDataHeight = testImages[i].rows;
            status = preProcCfg.getConfig(cmdArgs.modelDirectory);
            if (status < 0)
            {
                DL_INFER_LOG_ERROR("[%s:%d] ti::utils::getConfig() failed.\n",
                                    __FUNCTION__, __LINE__);
                exit(-1);
            }

            if (postProcessConfig.taskType == "segmentation")
            {
                postProcessConfig.inDataWidth  = ifOutInfo->shape[ifOutInfo->dim - 1];
                postProcessConfig.inDataHeight = ifOutInfo->shape[ifOutInfo->dim - 2];
                postProcessConfig.alpha        = cmdArgs.alpha;
            }

            else
            {
                postProcessConfig.inDataWidth  = preProcCfg.outDataWidth;
                postProcessConfig.inDataHeight = preProcCfg.outDataHeight;

                if (postProcessConfig.taskType == "classification")
                {
                    postProcessConfig.topN       = cmdArgs.topN;
                }
                else
                {
                    postProcessConfig.vizThreshold = cmdArgs.vizThreshold;
                }
            }

            /* Build Post Process Object. */
            PostprocessImageConfig   postProcCfg(postProcessConfig);

            postProcCfg.outDataWidth  = testImages[i].cols;
            postProcCfg.outDataHeight = testImages[i].rows;

            PostprocessImage *postProcObj;
            postProcObj = PostprocessImage::makePostprocessImageObj(postProcCfg);
            if (postProcObj == nullptr)
            {
                DL_INFER_LOG_ERROR("makePostprocessImageObj() failed.\n");
                exit(-1);
            }

            printf("\n[INFO] Image Width = %d, Image Height = %d" ,
                                         testImages[i].cols,testImages[i].rows);

            void        *inBuff;
            void        *ogBuff;
            cv::Mat     preProcImage;
            cv::Mat     nv12Image;
            cv::Mat     bgrImage;


            /* Get NV12 Image for post process*/
            nv12Image = cv::Mat::zeros(cv::Size(testImages[i].cols,testImages[i].rows*1.5),CV_8UC1);
            convertBGRtoNV12(testImages[i],nv12Image);

            /* Pre Process the image for dl inference. */
            preProcessImage(testImages[i],preProcImage,preProcCfg);

            /* Make Inferer. */
            InferencePipe *inferPipe = new InferencePipe(inferer,postProcObj,preProcCfg);

            inBuff = (void*)(preProcImage.data);
            ogBuff = (void*)(nv12Image.data);

            inferPipe->runModel(inBuff,ogBuff);

            string imgName = postProcCfg.taskType
                            + "_output"
                            + to_string(i)
                            +"_"
                            + postProcCfg.modelName
                            + ".jpg";

            /* Convert NV12 to BGR and Save */
            cv::cvtColor(nv12Image,bgrImage,cv::COLOR_YUV2BGR_NV12);
            cv::imwrite(imgName,bgrImage);
            delete inferPipe;
            delete postProcObj;
        }

        printf("\n");
        delete inferer;
    }

    return status;
}

