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

#ifndef _TI_POST_PROCESS_SEMANTIC_SEGMENTATION_
#define _TI_POST_PROCESS_SEMANTIC_SEGMENTATION_

/* Module headers. */
#include <ti_post_process.h>

/**
 * \defgroup group_post_process_semantic_segmentation Semantic Segmentation post-processing
 *
 * \brief Class implementing the semantic segmentation post-processing logic.
 *
 * \ingroup group_post_process
 */

namespace ti::post_process
{
    /** Post-processing for image based semantic segmentation
     *
     * \ingroup group_post_process_semantic_segmentation
     */
    class PostprocessSemanticSegmentation : public PostprocessImage
    {
        public:
            /** Constructor.
             *
             * @param config Configuration information not present in YAML
             */
            PostprocessSemanticSegmentation(const PostprocessImageConfig    &config);

            /** Function operator
             *
             * This is the heart of the class. The application uses this
             * interface to execute the functionality provided by this class.
             *
             * @param frameData Input data frame on which results are overlaid
             * @param results Segmentation output results from the inference
             */
            void *operator()(void               *frameData,
                             VecDlTensorPtr     &results,
                             PostProcessResult  *postProcessResult = NULL);

            /** Destructor. */
            ~PostprocessSemanticSegmentation();
        private:
            /**
             * Assignment operator.
             *
             * Assignment is not required and allowed and hence prevent
             * the compiler from generating a default assignment operator.
             */
            PostprocessSemanticSegmentation &
                operator=(const PostprocessSemanticSegmentation& rhs) = delete;

            /**
             * Copy constructor
             *
             * Copy constructor is not required and allowed and hence prevent
             * the compiler from generating a default copy constructor.
             */
            PostprocessSemanticSegmentation
                          (const PostprocessSemanticSegmentation& rhs) = delete;
    };

} // namespace ti::post_process

#endif /* _TI_POST_PROCESS_SEMANTIC_SEGMENTATION_ */
