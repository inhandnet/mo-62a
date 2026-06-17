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

#ifndef _TI_POST_PROCESS_IMAGE_CLASSIFICATION_
#define _TI_POST_PROCESS_IMAGE_CLASSIFICATION_

/* Module headers. */
#include <ti_post_process.h>

/**
 * \defgroup group_post_process_img_classification Image Classification post-processing
 *
 * \brief Class implementing the image classification post-processing logic.
 *
 * \ingroup group_post_process
 */

namespace ti::post_process
{
    /** Post-processing for image classification.
     *
     * \ingroup group_post_process_img_classification
     */
    class PostprocessImageClassification : public PostprocessImage
    {
        public:
            /** Constructor.
             *
             * @param config Configuration information not present in YAML
             */
            PostprocessImageClassification(const PostprocessImageConfig   &config);

            /** Function operator
             *
             * This is the heart of the class. The application uses this
             * interface to execute the functionality provided by this class.
             *
             * @param frameData Input data frame on which results are overlaid
             * @param results Classification output results from the inference
             * @param postProcessResult (Optional) Structure to fill post process info
             */
            void *operator()(void               *frameData,
                             VecDlTensorPtr     &results,
                             PostProcessResult  *postProcessResult = NULL);

            /** Destructor. */
            ~PostprocessImageClassification();

        private:
            /** Structure to hold information about NV12 Image. */
            Image           m_imageHolder;

            /** Color of the title Text. */
            YUVColor        m_titleColor;

            /** Color of the result text. */
            YUVColor        m_textColor;

            /** Color of the background for text. */
            YUVColor        m_textBgColor;

            /** Font of title. */
            FontProperty    m_titleFont;

            /** Font of result text. */
            FontProperty    m_textFont;

        private:
            /**
             * Assignment operator.
             *
             * Assignment is not required and allowed and hence prevent
             * the compiler from generating a default assignment operator.
             */
            PostprocessImageClassification &
                operator=(const PostprocessImageClassification& rhs) = delete;
    };

} // namespace ti::post_process

#endif /* _TI_POST_PROCESS_IMAGE_CLASSIFICATION_ */

