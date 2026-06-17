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
#include <ti_post_process_image_classification.h>

namespace ti::post_process
{
using namespace std;

#define INVOKE_OVERLAY_CLASS_LOGIC(T)                    \
    overlayTopNClasses(frameData,                        \
                       reinterpret_cast<T*>(buff->data), \
                       postProcessResult,                \
                       m_config.datasetInfo,             \
                       labelOffset,                      \
                       m_config.topN,                    \
                       buff->numElem,                    \
                       &m_imageHolder,                   \
                       &m_titleColor,                    \
                       &m_textColor,                     \
                       &m_textBgColor,                   \
                       &m_titleFont,                     \
                       &m_textFont)

PostprocessImageClassification::PostprocessImageClassification(const PostprocessImageConfig &config):
    PostprocessImage(config)
{
    m_imageHolder.width = config.outDataWidth;
    m_imageHolder.height = config.outDataHeight;

    /** Get YUV value for green color. */
    getColor(&m_titleColor,0,255,0);

    /** Get YUV value for yellow color. */
    getColor(&m_textColor, 255, 255, 0);

    /** Get YUV value for dark blue color. */
    getColor(&m_textBgColor, 5, 11, 120);

    /** Get Monospace font from available font sizes
     *  where width of character is closest to 2.5%
     *  of the total width image width
     */
    int titleSize  = (int)(0.015*config.outDataWidth);
    getFont(&m_titleFont,titleSize);
    
    /** Get Monospace font from available font sizes
     *  where width of character is closest to 1.5%
     *  of the total width image width
     */
    int textSize  = (int)(0.015*config.outDataWidth);
    getFont(&m_textFont,textSize);
}

/**
 * Extract the top classes in decreasing order, from the data, 
 * in order to create an argmax tuple
 * A normal sort would have destroyed the original information regarding the
 * index of a certain value. This function returns a vector of a tuple containing
 * both the value and index respectively.
 *
 * @param data An array of data to sort.
 * @param size Number of elements in the input array.
 * @returns Top  values sorted vector containing a tuple of the value and
 *          original index
 */
template <typename T>
static vector<tuple<T, int32_t>> get_argmax_sorted(T       *data,
                                                   int32_t  size)
{
    vector<tuple<T, int32_t>> argmax;

    for (int i = 0; i < size; i++)
    {
        argmax.push_back(make_tuple(data[i], i));
    }

    sort(argmax.rbegin(), argmax.rend());
    return argmax;
}

/**
 * Extract the top N classes in decreasing order, from the data, 
 * in order to create an argmax tuple
 * A normal sort would have destroyed the original information regarding the
 * index of a certain value. This function returns a vector of a tuple containing
 * both the value and index respectively.
 *
 * @param data An array of data to sort.
 * @param size Number of elements in the input array.
 * @returns Top N values sorted vector containing a tuple of the value and
 *          original index
 *          if N > the size of the results vector then an empty vector
 *          is returned.
 */
template <typename T>
static vector<tuple<T, int32_t>> get_topN(T        *data,
                                          int32_t   N,
                                          int32_t   size)
{
    vector<tuple<T, int32_t>> argmax;

    if (N == size)
    {
        return get_argmax_sorted<T>(data, size);
    }
    else if (N < size)
    {
        for (int32_t i = 0; i < N; i++)
        {
            argmax.push_back(make_tuple(data[i], i));
        }

        sort(argmax.rbegin(), argmax.rend());

        for (int32_t i = N; i < size; i++)
        {
            if (get<0>(argmax[N-1]) < data[i])
            {
                argmax[N-1] = make_tuple(data[i], i);
                sort(argmax.rbegin(), argmax.rend());
            }
        }
    }

    return argmax;
}

/**
  * @param frame Original NV12 data buffer, where the in-place updates will happen
  * @param results Reference to a vector of vector of floats representing the output
  *          from an inference API. It should contain 1 vector representing the
  *          probability with which that class is detected in this image.
  * @param size Number of elements in the input array 'results'.
  * @returns original frame with some in-place post processing done
  */
template <typename T1, typename T2>
static T1 *overlayTopNClasses(T1                        *frame,
                              T2                        *results,
                              PostProcessResult         *postProcessResult,
                              map<int32_t,DatasetInfo>  datasetInfo,
                              int32_t                   labelOffset,
                              int32_t                   N,
                              int32_t                   size,
                              Image                     *imgHolder,
                              YUVColor                  *titleColor,
                              YUVColor                  *textColor,
                              YUVColor                  *textBgColor,
                              FontProperty              *titleFont,
                              FontProperty              *textFont
                              )
{
    vector<tuple<T2,int32_t>> argmax;
    std::string title = "Recognized Classes (Top " + std::to_string(N) + "):\0";
    int titleYPos = (int)(0.075 * imgHolder->height);
    int yOffset = (titleFont->height) + 12;

    argmax = get_topN<T2>(results, N, size);

    if (NULL != frame)
    {
        imgHolder->yRowAddr = (uint8_t *)frame;
        imgHolder->uvRowAddr = (uint8_t *)frame + (imgHolder->width*imgHolder->height);

        drawRect(imgHolder,
                0,
                titleYPos,
                (titleFont->width * title.length()) + 10,
                yOffset,
                textBgColor,
                -1);
        drawText(imgHolder,title.c_str(),5,titleYPos,titleFont,titleColor);
    }


    for (int i = 0; i < N; i++)
    {
        int32_t index = get<1>(argmax[i]) + labelOffset;

        if (index >= 0)
        {
            string str;

            if (datasetInfo.find(index) != datasetInfo.end())
            {
                str = datasetInfo.at(index).name;
                if ("" != datasetInfo.at(index).superCategory)
                {
                    str = datasetInfo.at(index).superCategory + "/" + str;
                }
            }
            else
            {
                str = "UNDEFINED";
            }

            if (NULL != frame)
            {
                int32_t row = (i*textFont->height) + yOffset;
                drawRect(imgHolder,
                        0,
                        titleYPos+row,
                        (textFont->width * str.length()) + 10,
                        textFont->height,
                        textBgColor,
                        -1);
                drawText(imgHolder,str.c_str(),5,titleYPos+row,textFont,textColor);
            }

            if (NULL != postProcessResult)
            {
                postProcessResult->m_imgClResult.m_label.push_back(str);
                postProcessResult->m_imgClResult.m_labelId.push_back(index);
            }
        }
    }

    return frame;
}

void *PostprocessImageClassification::operator()(void               *frameData,
                                                 VecDlTensorPtr     &results,
                                                 PostProcessResult  *postProcessResult)
{
    /* Even though a vector of variants is passed only the first
     * entry is valid.
     */
    auto       *buff = results[0];
    void       *ret = frameData;
    int32_t     labelOffset = m_config.labelOffsetMap.at(0);

    if (NULL != postProcessResult)
    {
        postProcessResult->m_inputWidth = m_config.inDataWidth;
        postProcessResult->m_inputHeight = m_config.inDataHeight;
        postProcessResult->m_outputWidth = m_config.outDataWidth;
        postProcessResult->m_outputHeight = m_config.outDataHeight;
        postProcessResult->m_imgClResult.m_label.clear();
        postProcessResult->m_imgClResult.m_labelId.clear();
    }

    if (buff->type == DlInferType_Int8)
    {
        ret = INVOKE_OVERLAY_CLASS_LOGIC(int8_t);
    }
    else if (buff->type == DlInferType_UInt8)
    {
        ret = INVOKE_OVERLAY_CLASS_LOGIC(uint8_t);
    }
    else if (buff->type == DlInferType_Int16)
    {
        ret = INVOKE_OVERLAY_CLASS_LOGIC(int16_t);
    }
    else if (buff->type == DlInferType_UInt16)
    {
        ret = INVOKE_OVERLAY_CLASS_LOGIC(uint16_t);
    }
    else if (buff->type == DlInferType_Int32)
    {
        ret = INVOKE_OVERLAY_CLASS_LOGIC(int32_t);
    }
    else if (buff->type == DlInferType_UInt32)
    {
        ret = INVOKE_OVERLAY_CLASS_LOGIC(uint32_t);
    }
    else if (buff->type == DlInferType_Int64)
    {
        ret = INVOKE_OVERLAY_CLASS_LOGIC(int64_t);
    }
    else if (buff->type == DlInferType_Float32)
    {
        ret = INVOKE_OVERLAY_CLASS_LOGIC(float);
    }

    return ret;
}

PostprocessImageClassification::~PostprocessImageClassification()
{
}

} // namespace ti::post_process
