/*
 *
 * Copyright (c) 2022 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#if !defined(_TI_TFLITE_INFERER_)
#define _TI_TFLITE_INFERER_

/* Third-party headers. */
#include <tensorflow/lite/model.h>
#include <tensorflow/lite/interpreter.h>
#include <tensorflow/lite/kernels/register.h>

/* Module headers. */
#include <ti_dl_inferer.h>

/**
 * \defgroup group_tflite_inferer TFLITE runtime API
 *
 * \brief A class for encapsulating the TFLITE runtime API.
 * \ingroup group_dl_inferer
 */

namespace ti::dl_inferer
{
    /** \brief A concrete class for TFLITE RT API.
     *
     * \ingroup group_tflite_inferer
     */
    class TFLiteInferer: public DLInferer
    {
        public:
            /**
             * Constructor.
             *
             * @param config InfererConfig.
             */
            TFLiteInferer(const InfererConfig &config);

            /**
             * Runs the model. This should be called only after all the inputs
             * have been set using setInput() call(s).
             *
             * @param inputs Input buffers to set for inference run
             * @param outputs Output buffers to set for inference run
             *
             * @returns 0 upon success. A nagative value otherwise.
             */
            virtual int32_t run(const VecDlTensorPtr &inputs,
                                VecDlTensorPtr       &outputs);

            /**
             * Dumps the model information to the screen.
             */
            virtual void dumpInfo();

            /**
             * Returns a pointer to an array containing detailed information on
             * the inputs of the model.
             *
             * @returns A pointer to an array of input interface parameters.
             */
            virtual const VecDlTensor *getInputInfo();

            /**
             * Returns an array containing detailed information on the outputs
             * of the model.
             *
             * @returns A pointer to an array of output interface parameters.
             */
            virtual const VecDlTensor *getOutputInfo();

            /**
             * Returns an allocated pointer that can be consumed by inference
             * of the model by this framework.
             *
             * @returns An pointer to allocated memory.
             */
            virtual void *allocate(int64_t size) override;

            /**
             * Destructor
             */
            ~TFLiteInferer();

        private:
            /** Path to the model. */
            std::string                                 m_modelPath;

            /** Path to the directory containing the model artifacts. */
            std::string                                 m_artifactPath;

            /** Enable TIDL. */
            bool                                        m_enableTidl;

            /** Core Number. */
            int                                         m_coreNumber;

            /** Debug Level. */
            int                                         m_debug_level;

            /**  A pointer to the model representation in memory. */
            std::unique_ptr<tflite::FlatBufferModel>    m_model;

            /** TODO. */
            tflite::ops::builtin::BuiltinOpResolver     m_resolver;

            /** A pointer to the model interpreter. */
            std::unique_ptr<tflite::Interpreter>        m_interpreter;
            
            /** Input tensor count. */
            uint32_t                                    m_numInputs;

            /** Output tensor count. */
            uint32_t                                    m_numOutputs;

            /** A list of input interface details. */
            VecDlTensor                                 m_inputs;

            /** A list of output interface details. */
            VecDlTensor                                 m_outputs;

        private:
            /**
             * Quesries the model and extracts the details of the input parameters.
             *
             * @returns 0 upon success. A negative value otherwise.
             */
            int32_t populateInputInfo();

            /**
             * Quesries the model and extracts the details of the output parameters.
             *
             * @returns 0 upon success. A negative value otherwise.
             */
            int32_t populateOutputInfo();
    };

} // namespace ti::dl_inferer

#endif // _TI_TFLITE_INFERER_

