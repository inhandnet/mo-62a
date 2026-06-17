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

#ifndef _TI_POST_PROCESS_OBJECT_DETECTION_
#define _TI_POST_PROCESS_OBJECT_DETECTION_

/* Module headers. */
#include <ti_post_process.h>

/**
 * \defgroup group_post_process_obj_detection Object Detection post-processing
 *
 * \brief Class implementing the object detection post-processing logic.
 *
 * \ingroup group_post_process
 */

namespace ti::post_process
{
    /** Post-processing for image based object detection.
     *
     * \ingroup group_post_process_obj_detection
     */
    class PostprocessObjectDetection : public PostprocessImage
    {
        public:
            /** Constructor.
             *
             * @param config Configuration information not present in YAML
             */
            PostprocessObjectDetection(const PostprocessImageConfig  &config);

            /** Function operator
             *
             * This is the heart of the class. The application uses this
             * interface to execute the functionality provided by this class.
             *
             * @param frameData Input data frame on which results are overlaid
             * @param results Detection output results from the inference
             * @param PostProcessResult (Optional) Structure to fill post process info
             */
            void *operator()(void               *frameData,
                             VecDlTensorPtr     &results,
                             PostProcessResult  *postProcessResult = NULL);

            /** Destructor. */
            ~PostprocessObjectDetection();

        private:
            /** Multiplicative factor to be applied to X co-ordinates. */
            float                   m_scaleX{1.0f};

            /** Multiplicative factor to be applied to Y co-ordinates. */
            float                   m_scaleY{1.0f};

            /** Structure to hold information about NV12 Image. */
            Image                   m_imageHolder;

            /** Color of the bounding box. */
            YUVColor                m_boxColor;

            /** Color of the class text. */
            YUVColor                m_textColor;

            /** Color of the text background. */
            YUVColor                m_textBGColor;

            /** Font of the class text. */
            FontProperty            m_textFont;

        private:
            /**
             * Assignment operator.
             *
             * Assignment is not required and allowed and hence prevent
             * the compiler from generating a default assignment operator.
             */
            PostprocessObjectDetection &
                operator=(const PostprocessObjectDetection& rhs) = delete;
    };

} // namespace ti::post_process

#endif /* _TI_POST_PROCESS_OBJECT_DETECTION_ */
