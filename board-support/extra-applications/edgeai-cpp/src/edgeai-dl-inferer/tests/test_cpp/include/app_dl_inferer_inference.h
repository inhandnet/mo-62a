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

#ifndef _APP_DL_INFERER_INFERENCE_H_
#define _APP_DL_INFERER_INFERENCE_H_

/* Standard headers. */
#include <thread>

/* Module headers. */
#include <test_cpp/include/app_dl_inferer_utils.h>

/* DL Inferer. */
#include <ti_dl_inferer.h>
#include <ti_pre_process_config.h>
#include <ti_post_process.h>
#include <ti_dl_inferer_logger.h>

/**
 * \defgroup group_dl_inferer_cpp_test Master demo code
 *
 * \brief Common code used across all EdgeAI applications.
 *
 * \ingroup group_dl_inferer
 */

namespace ti::app_dl_inferer::common
{
    using namespace std;
    using namespace ti::dl_inferer;
    using namespace ti::dl_inferer::utils;
    using namespace ti::post_process;
    using namespace ti::pre_process;
    using namespace ti::app_dl_inferer::common;
    /**
     * \brief Main class that integrates the pre-processing, DL inferencing, and
     *        post-processing operations.
     *
     *        It consumes the input coming from a gstreamer pipeline setup outside
     *        the scope of the class and feeds another gstreamer pipeline for
     *        consuming the results of the post-processing.
     *
     * \ingroup group_dl_inferer_cpp_test
     */
    class InferencePipe
    {
        public:
            /** Constructor.
             *
             * @param infererObj Inference context
             * @param postProcObj Post-processing object
             * @param preProcConfig Pre-processing config
             */
            InferencePipe(DLInferer                 *infererObj,
                          PostprocessImage          *postProcObj,
                          PreprocessImageConfig     &preProcConfig);

            int32_t getInstId();

            /**
             * Run the inference model with provided input data and save the
             * results in the referenced vector of vector
             *
             * @param inputBuff input data
             * @param originalBuff original frame for post Processing
             * @returns zero on success, non-zero on failure
             */
            int runModel(void *inputBuff,void *originalBuff);

            /** Destructor. */
            ~InferencePipe();

        private:
            /**
             * Assignment operator.
             *
             * Assignment is not required and allowed and hence prevent
             * the compiler from generating a default assignment operator.
             */
            InferencePipe & operator=(const InferencePipe& rhs) = delete;

        private:

            /** Inference context. */
            DLInferer              *m_inferer{nullptr};

            /** Pre-processing config. */
            PreprocessImageConfig   m_preProcCfg;

            /** Post-processing context. */
            PostprocessImage       *m_postProcObj{nullptr};

            /** Number of inputs from the inference processing. */
            int32_t                 m_numInputs;

            /** Number of outputs from the inference processing. */
            int32_t                 m_numOutputs;

            /** Inference thread identifier. */
            thread                  m_inferThreadId;

            /** Input buffers to the inference. */
            VecDlTensorPtr          m_inferInputBuff;

            /** Output buffers to the inference. */
            VecDlTensorPtr          m_inferOutputBuff;

            /** Frame rate of the input data. */
            uint32_t                m_frameRate;

            /** Instance Id. */
            uint32_t                m_instId{};

            /** Instance count. */
            static uint32_t         m_instCnt;

            /** Flag to control the execution. */
            bool                    m_running;

    };

} // namespace ti::app_dl_inferer::common

#endif /* _APP_DL_INFERER_INFERENCE_H_ */
