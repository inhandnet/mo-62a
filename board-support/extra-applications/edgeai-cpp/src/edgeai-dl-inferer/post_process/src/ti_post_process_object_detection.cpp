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
#include <ti_post_process_object_detection.h>

namespace ti::post_process
{
using namespace std;
PostprocessObjectDetection::PostprocessObjectDetection(const PostprocessImageConfig   &config):
    PostprocessImage(config)
{
    if (m_config.normDetect)
    {
        m_scaleX = static_cast<float>(m_config.outDataWidth);
        m_scaleY = static_cast<float>(m_config.outDataHeight);
    }
    else
    {
        m_scaleX = static_cast<float>(m_config.outDataWidth)/m_config.inDataWidth;
        m_scaleY = static_cast<float>(m_config.outDataHeight)/m_config.inDataHeight;
    }

    m_imageHolder.width = m_config.outDataWidth;
    m_imageHolder.height = m_config.outDataHeight;
    getColor(&m_boxColor,20,220,20);
    getColor(&m_textColor,0,0,0);
    getColor(&m_textBGColor,0,255,0);
    getFont(&m_textFont,12);
}

/**
 * @param frame Original NV12 data buffer, where the in-place updates will happen
 * @param box bounding box co-ordinates.
 * @param outDataWidth width of the output buffer.
 * @param outDataHeight Height of the output buffer.
 *
 * @returns original frame with some in-place post processing done
 */
static void overlayBoundingBox(Image                        *img,
                               int                          *box,
                               const std::string            objectname,
                               YUVColor                     *boxColor,
                               YUVColor                     *textColor,
                               YUVColor                     *textBGColor,
                               FontProperty                 *textFont
                               )
{
    // Draw bounding box for the detected object
    drawRect(img,
             box[0],
             box[1],
             box[2] - box[0],
             box[3] - box[1],
             boxColor,
             2);

    drawRect(img,
             (box[0] + box[2])/2 - 5,
             (box[1] + box[3])/2 - 5,
             objectname.size() * textFont->width + 10,
             textFont->height + 10,
             textBGColor,
             -1);

    drawText(img,
             objectname.c_str(),
             (box[0] + box[2])/2,
             (box[1] + box[3])/2,
             textFont,
             textColor);
}

void *PostprocessObjectDetection::operator()(void               *frameData,
                                            VecDlTensorPtr      &results,
                                            PostProcessResult   *postProcessResult)
{
    /* The results has three vectors. We assume that the type
     * of all these is the same.
     */
    std::vector<int64_t>    lastDims;
    VecDlTensorPtr          resultRo;
    int32_t                 ignoreIndex;
    void                    *ret     = frameData;

    if (NULL != postProcessResult)
    {
        postProcessResult->m_inputWidth = m_config.inDataWidth;
        postProcessResult->m_inputHeight = m_config.inDataHeight;
        postProcessResult->m_outputWidth = m_config.outDataWidth;
        postProcessResult->m_outputHeight = m_config.outDataHeight;
        postProcessResult->m_objDetResult.m_label.clear();
        postProcessResult->m_objDetResult.m_labelId.clear();
        postProcessResult->m_objDetResult.m_score.clear();
        postProcessResult->m_objDetResult.m_box.clear();
    }

    /* Extract the last dimension from each of the output
     * tensors.
     * last dimension will give the number of values present
     * in given tensor
     * Ex: if shape of a tensor is
     *  [1][1][100][4] -> there are 4 values in the given tensor and 100 entries
     *  [100]          -> 1 value in given tensor and 100 entries, should not
     *                    consider last dim when number of dim is 1
     * Need to ignore all dimensions with value 1 since it does not actually add
     * a dimension (this is similar to squeeze operation in numpy)
     */
    for (uint64_t i = 0; i < results.size(); i++)
    {
        auto   *result = results[m_config.resultIndices[i]];
        auto   &shape = result->shape;
        auto    nDims = result->dim;

        resultRo.push_back(result);

        for (auto s: shape)
        {
           if (s == 1)
           {
               nDims--;
           }
        }

        if (nDims == 1)
        {
            lastDims.push_back(1);
        }
        else
        {
            lastDims.push_back(result->shape[result->dim - 1]);
        }
    }

    ignoreIndex = m_config.ignoreIndex;

    auto getVal = [&ignoreIndex, &lastDims, &resultRo] (int32_t iter, int32_t pos)
    {
        int64_t cumuDims = 0;

        for (uint64_t i=0; i < lastDims.size(); i++)
        {
            cumuDims += lastDims[i];
            if (ignoreIndex != -1 && pos >= ignoreIndex)
                pos++;
            auto offset = iter * lastDims[i] + pos - cumuDims + lastDims[i];

            if (pos < cumuDims)
            {
                if (resultRo[i]->type == DlInferType_Int8)
                {
                    return (float)reinterpret_cast<int8_t*>(resultRo[i]->data)[offset];
                }
                else if (resultRo[i]->type == DlInferType_UInt8)
                {
                    return (float)reinterpret_cast<uint8_t*>(resultRo[i]->data)[offset];
                }
                else if (resultRo[i]->type == DlInferType_Int16)
                {
                    return (float)reinterpret_cast<int16_t*>(resultRo[i]->data)[offset];
                }
                else if (resultRo[i]->type == DlInferType_UInt16)
                {
                    return (float)reinterpret_cast<uint16_t*>(resultRo[i]->data)[offset];
                }
                else if (resultRo[i]->type == DlInferType_Int32)
                {
                    return (float)reinterpret_cast<int32_t*>(resultRo[i]->data)[offset];
                }
                else if (resultRo[i]->type == DlInferType_UInt32)
                {
                    return (float)reinterpret_cast<uint32_t*>(resultRo[i]->data)[offset];
                }
                else if (resultRo[i]->type == DlInferType_Int64)
                {
                    return (float)reinterpret_cast<int64_t*>(resultRo[i]->data)[offset];
                }
                else if (resultRo[i]->type == DlInferType_Float32)
                {
                    return (float)reinterpret_cast<float*>(resultRo[i]->data)[offset];
                }
            }
        }

        return (float)0;
    };

    m_imageHolder.yRowAddr = (uint8_t *)frameData;
    m_imageHolder.uvRowAddr = (uint8_t *)frameData + (m_imageHolder.width*m_imageHolder.height);

    int32_t numEntries = resultRo[0]->numElem/lastDims[0];

    for (auto i = 0; i < numEntries; i++)
    {
        float score;
        int label;
        int box[4];
        int adj_class_id;
        std::string objectname;

        score = getVal(i, m_config.formatter[5]);

        if (score < m_config.vizThreshold)
        {
            continue;
        }
        
        box[0] = getVal(i, m_config.formatter[0]) * m_scaleX;
        box[1] = getVal(i, m_config.formatter[1]) * m_scaleY;
        box[2] = getVal(i, m_config.formatter[2]) * m_scaleX;
        box[3] = getVal(i, m_config.formatter[3]) * m_scaleY;

        label = getVal(i, m_config.formatter[4]);

        if (m_config.labelOffsetMap.find(label) != m_config.labelOffsetMap.end())
        {
            adj_class_id = m_config.labelOffsetMap.at(label);
        }
        else
        {
            adj_class_id = m_config.labelOffsetMap.at(0) + label;
        }

        if (m_config.datasetInfo.find(adj_class_id) != m_config.datasetInfo.end())
        {
            objectname = m_config.datasetInfo.at(adj_class_id).name;
            if ("" != m_config.datasetInfo.at(adj_class_id).superCategory)
            {
                objectname = m_config.datasetInfo.at(adj_class_id).superCategory +
                             "/" +
                             objectname;
            }

            m_boxColor.Y = m_config.datasetInfo.at(adj_class_id).yuvColor[0];
            m_boxColor.U = m_config.datasetInfo.at(adj_class_id).yuvColor[1];
            m_boxColor.V = m_config.datasetInfo.at(adj_class_id).yuvColor[2];
            m_textBGColor.Y = m_boxColor.Y;
            m_textBGColor.U = m_boxColor.U;
            m_textBGColor.V = m_boxColor.V;
            if(m_textBGColor.Y >= 128 )
            {
                getColor(&m_textColor, 0, 0, 0);
            }
            else
            {
                getColor(&m_textColor, 255, 255, 255);
            }
        }
        else
        {
            objectname = "UNDEFINED";
        }

        if (NULL != frameData)
        {
            overlayBoundingBox( &m_imageHolder, box, objectname,
                                &m_boxColor, &m_textColor, &m_textBGColor,
                                &m_textFont);
        }

        if (NULL != postProcessResult)
        {
            postProcessResult->m_objDetResult.m_label.push_back(objectname);
            postProcessResult->m_objDetResult.m_labelId.push_back(adj_class_id);
            postProcessResult->m_objDetResult.m_score.push_back(score);
            postProcessResult->m_objDetResult.m_box.push_back({
                box[0], box[1], box[2], box[3]
            });
        }
    }

    return ret;
}

PostprocessObjectDetection::~PostprocessObjectDetection()
{
}

} // namespace ti::post_process