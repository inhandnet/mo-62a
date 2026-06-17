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

#ifndef _APP_DL_INFERER_UTILS_H_
#define _APP_DL_INFERER_UTILS_H_

/* Third Party Headers. */
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

/* Module Headers. */
#include <ti_pre_process_config.h>

namespace ti::app_dl_inferer::common
{
    using namespace ti::pre_process;
    using namespace std;

    /**
     * Resizes, Crops and Color Converts the image based on Config file
     *
     * @param testImage    Original Image.
     * @param preProcImage PreProcessed Image
     * @param preProcCfg   PrePocess Config
     *
     */
    
    void preProcessImage(cv::Mat               &testImage,
                         cv::Mat               &preProcImage,
                         PreprocessImageConfig &preProcCfg);

     /**
     * Convert BGR Image to NV12
     *
     * @param bgrMat  Original BGR Image.
     * @param nv12Mat NV12 Image
     *
     */
    void convertBGRtoNV12(const cv::Mat& bgrMat, cv::Mat& nv12Mat);

} // namespace ti::app_dl_inferer::common

#endif /* _APP_DL_INFERER_UTILS_H_ */
