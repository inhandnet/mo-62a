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

/* Module headers. */
#include <ti_post_process_semantic_segmentation.h>

/* Default YUV Color map. */
uint8_t YUV_COLOR_MAP[26][3] = {{0,128,128},{149,43,21},{92,127,114},
                                {255,0,148},{178,171,0},{86,217,66},
                                {150,42,202},{161,93,13},{152,108,187},
                                {93,126,233},{168,127,92},{105,212,234},
                                {215,150,139},{214,141,156},{64,91,81},
                                {29,255,107},{32,109,183},{97,73,136},
                                {35,136,173},{62,92,147},{76,84,255} ,
                                {79,226,192},{210,9,123},{78,155,127},
                                {126,98,132},{12,183,119}};

namespace ti::post_process
{
#define INVOKE_BLEND_LOGIC(T)                           \
    blendSegMask(reinterpret_cast<uint8_t*>(frameData), \
                 reinterpret_cast<T*>(buff->data),      \
                 postProcessResult,                     \
                 m_config.inDataWidth,                  \
                 m_config.inDataHeight,                 \
                 m_config.outDataWidth,                 \
                 m_config.outDataHeight,                \
                 m_config.alpha,                        \
                 m_config.datasetInfo)

PostprocessSemanticSegmentation::PostprocessSemanticSegmentation(const PostprocessImageConfig   &config):
    PostprocessImage(config)
{

}

/**
 * For every pixel in input frame, this will find the scaled co-ordinates for a
 * downscaled result and use the color associated with detected class ID.
 *
 * @param frame Original NV12 data buffer, where the in-place updates will happen
 * @param classes Reference to a vector of vector of floats representing the output
 *          from an inference API. It should contain 1 vector describing the class ID
 *          detected for that pixel.
 * @returns original frame with some in-place post processing done
 */
template <typename T1, typename T2>
static T1 *blendSegMask(T1                              *frame,
                        T2                              *classes,
                        PostProcessResult               *postProcessResult,
                        int32_t                         inDataWidth,
                        int32_t                         inDataHeight,
                        int32_t                         outDataWidth,
                        int32_t                         outDataHeight,
                        float                           alpha,
                        std::map<int32_t, DatasetInfo>  datasetInfo)
{
    uint8_t     a;
    uint8_t     sa;
    uint8_t*    uvPtr;
    uint8_t     y_m;
    uint8_t     u_m;
    uint8_t     v_m;
    int32_t     w;
    int32_t     h;
    int32_t     sw;
    int32_t     sh;
    int32_t     class_id;
    int32_t     rowOffset;

    a  = alpha * 256;
    sa = (1 - alpha ) * 256;
    int     uvOffset = outDataHeight*outDataWidth;

    uint32_t maxClass = sizeof(YUV_COLOR_MAP)/sizeof(YUV_COLOR_MAP[0]);

    if (NULL != postProcessResult)
    {
        int32_t res = (outDataHeight/2)*(outDataWidth/2);
        postProcessResult->m_semSegResult.m_classId.reserve(res);
    }

    // Here, (w, h) iterate over frame and (sw, sh) iterate over classes
    for (h = 0; h < outDataHeight/2; h++)
    {
        sh = (int32_t)((h << 1) * inDataHeight / outDataHeight);
        uvPtr = frame + uvOffset + (h * outDataWidth);
        rowOffset = sh*inDataWidth;

        for (w = 0; w < outDataWidth; w+=2)
        {
            int32_t index;

            sw = (int32_t)(w * inDataWidth / outDataWidth);
            index = (int32_t)(rowOffset + sw);
            class_id =  classes[index];

            u_m = 128;
            v_m = 128;


            if (datasetInfo.find(class_id) != datasetInfo.end())
            {
                u_m = datasetInfo.at(class_id).yuvColor[1];
                v_m = datasetInfo.at(class_id).yuvColor[2];
            }
            else if (class_id < maxClass)
            {
                u_m = YUV_COLOR_MAP[class_id][1];
                v_m = YUV_COLOR_MAP[class_id][2];
            }

            u_m = ((*(uvPtr) * a) + (u_m * sa)) >> 8;
            v_m = ((*(uvPtr+1) * a) + (v_m * sa)) >> 8;
            *((uint16_t*)uvPtr) = (v_m << 8) | u_m;
            uvPtr += 2;

            if (NULL != postProcessResult)
            {
                postProcessResult->m_semSegResult.m_classId.push_back(class_id);
            }

        }
    }
    return frame;
}

void *PostprocessSemanticSegmentation::operator()(void              *frameData,
                                                  VecDlTensorPtr    &results,
                                                  PostProcessResult *postProcessResult)
{
    /* Even though a vector of variants is passed only the first
     * entry is valid.
     */
    auto *buff = results[0];
    void *ret  = frameData;

    if (NULL != postProcessResult)
    {
        postProcessResult->m_inputWidth = m_config.inDataWidth;
        postProcessResult->m_inputHeight = m_config.inDataHeight;
        postProcessResult->m_outputWidth = m_config.outDataWidth;
        postProcessResult->m_outputHeight = m_config.outDataHeight;
        postProcessResult->m_semSegResult.m_classId.clear();
    }

    if (NULL != frameData)
    {
        if (buff->type == DlInferType_Int8)
        {
            ret = INVOKE_BLEND_LOGIC(int8_t);
        }
        else if (buff->type == DlInferType_UInt8)
        {
            ret = INVOKE_BLEND_LOGIC(uint8_t);
        }
        else if (buff->type == DlInferType_Int16)
        {
            ret = INVOKE_BLEND_LOGIC(int16_t);
        }
        else if (buff->type == DlInferType_UInt16)
        {
            ret = INVOKE_BLEND_LOGIC(uint16_t);
        }
        else if (buff->type == DlInferType_Int32)
        {
            ret = INVOKE_BLEND_LOGIC(int32_t);
        }
        else if (buff->type == DlInferType_UInt32)
        {
            ret = INVOKE_BLEND_LOGIC(uint32_t);
        }
        else if (buff->type == DlInferType_Int64)
        {
            ret = INVOKE_BLEND_LOGIC(int64_t);
        }
        else if (buff->type == DlInferType_Float32)
        {
            ret = INVOKE_BLEND_LOGIC(float);
        }
    }

    return ret;
}

PostprocessSemanticSegmentation::~PostprocessSemanticSegmentation()
{

}

} // namespace ti::post_process
