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
#include <test_cpp/include/app_dl_inferer_utils.h>
#include <ti_post_process.h>

namespace ti::app_dl_inferer::common
{

void preProcessImage(cv::Mat               &testImage,
                    cv::Mat                &preProcImage,
                    PreprocessImageConfig  &preProcCfg)
{
    cv::Mat resizedImage;

    /* Resize. */
    cv::resize(testImage,
               resizedImage,
               cv::Size(preProcCfg.resizeWidth, preProcCfg.resizeHeight),
               cv::INTER_LINEAR);

    if (!preProcCfg.reverseChannel)
    {
        cv::cvtColor(resizedImage, resizedImage, cv::COLOR_BGR2RGB);
    }

    /* Center Crop. */
    if (preProcCfg.resizeHeight != preProcCfg.outDataHeight &&
        preProcCfg.resizeWidth != preProcCfg.outDataWidth)
    {

        int32_t x_boundary = (preProcCfg.resizeWidth - preProcCfg.outDataWidth)/2;
        int32_t y_boundary = (preProcCfg.resizeHeight - preProcCfg.outDataHeight)/2;

        int32_t xMin = x_boundary;
        int32_t xMax = preProcCfg.resizeWidth-x_boundary;

        int32_t yMin = y_boundary;
        int32_t yMax = preProcCfg.resizeHeight-y_boundary;

        resizedImage(cv::Rect(xMin,yMin,xMax-xMin,yMax-yMin)).copyTo(preProcImage);
    }
    else
    {
        resizedImage.copyTo(preProcImage);
    }
}

void convertBGRtoNV12(const cv::Mat& bgrMat, cv::Mat& nv12Mat)
{
    int counter = 0;
    for (int ih=0;ih<bgrMat.rows;ih+=2)
    {
        const uint8_t* bgrMatRowPtr_0 = bgrMat.ptr<uint8_t>(ih);
        const uint8_t* bgrMatRowPtr_1 = bgrMat.ptr<uint8_t>(ih+1);
        uint8_t* nv12MatRowPtr_0 = nv12Mat.ptr<uint8_t>(ih);
        uint8_t* nv12MatRowPtr_1 = nv12Mat.ptr<uint8_t>(ih+1);

        int uv_row = counter+bgrMat.rows;
        uint8_t* nv12MatRowPtrUV = nv12Mat.ptr<uint8_t>(uv_row);
        counter++;

        for (int iw=0;iw<bgrMat.cols;iw+=2)
        {
            const int bgrMatColIdxBytes = iw*bgrMat.elemSize();
            const int nv12MatColIdxBytes = iw*nv12Mat.elemSize();
            const uint8_t B00 = bgrMatRowPtr_0[bgrMatColIdxBytes+0];
            const uint8_t G00 = bgrMatRowPtr_0[bgrMatColIdxBytes+1];
            const uint8_t R00 = bgrMatRowPtr_0[bgrMatColIdxBytes+2];
            const uint8_t B01 = bgrMatRowPtr_0[bgrMatColIdxBytes+3];
            const uint8_t G01 = bgrMatRowPtr_0[bgrMatColIdxBytes+4];
            const uint8_t R01 = bgrMatRowPtr_0[bgrMatColIdxBytes+5];

            const uint8_t B10 = bgrMatRowPtr_1[bgrMatColIdxBytes+0];
            const uint8_t G10 = bgrMatRowPtr_1[bgrMatColIdxBytes+1];
            const uint8_t R10 = bgrMatRowPtr_1[bgrMatColIdxBytes+2];
            const uint8_t B11 = bgrMatRowPtr_1[bgrMatColIdxBytes+3];
            const uint8_t G11 = bgrMatRowPtr_1[bgrMatColIdxBytes+4];
            const uint8_t R11 = bgrMatRowPtr_1[bgrMatColIdxBytes+5];

            const int Y00 = RGB2Y(R00,G00,B00);
            const int Y01 = RGB2Y(R01,G01,B01);
            const int Y10 = RGB2Y(R10,G10,B10);
            const int Y11 = RGB2Y(R11,G11,B11);

            const int U00 = RGB2U(R00,G00,B00);
            const int U01 = RGB2U(R01,G01,B01);
            const int U10 = RGB2U(R10,G10,B10);
            const int U11 = RGB2U(R11,G11,B11);

            const int V00 = RGB2V(R00,G00,B00);
            const int V01 = RGB2V(R01,G01,B01);
            const int V10 = RGB2V(R10,G10,B10);
            const int V11 = RGB2V(R11,G11,B11);

            const int avg_U = (U00+U01+U10+U11)>>2;
            const int avg_V = (V00+V01+V10+V11)>>2;;

            nv12MatRowPtr_0[nv12MatColIdxBytes]=cv::saturate_cast<uint8_t>(Y00);
            nv12MatRowPtr_0[nv12MatColIdxBytes+1]=cv::saturate_cast<uint8_t>(Y01);
            nv12MatRowPtr_1[nv12MatColIdxBytes]=cv::saturate_cast<uint8_t>(Y10);
            nv12MatRowPtr_1[nv12MatColIdxBytes+1]=cv::saturate_cast<uint8_t>(Y11);
            nv12MatRowPtrUV[nv12MatColIdxBytes] = cv::saturate_cast<uint8_t>(avg_U);
            nv12MatRowPtrUV[nv12MatColIdxBytes+1] = cv::saturate_cast<uint8_t>(avg_V);
        }
    }
}

} // namespace ti::app_dl_inferer::common