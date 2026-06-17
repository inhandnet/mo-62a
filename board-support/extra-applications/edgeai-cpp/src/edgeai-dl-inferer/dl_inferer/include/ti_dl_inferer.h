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
#if !defined(_TI_DL_INFERER_)
#define _TI_DL_INFERER_

/* Standard headers. */
#include <string>
#include <cstdarg>
#include <vector>
#include <mutex>
#include <stdexcept>

/* Module headers. */
#include <ti_dl_inferer_config.h>

/**
 * \defgroup group_dl_inferer Deep Learning Inference engine
 *
 * \brief Unified interface for running different DL run time APIs. The goal of
 *        this class to provide a common interface to the application for
 *        different runtime API (ex:- DLR, TFLITE,..). These underlying
 *        runtime libraries provide different capabilities and when designing
 *        the applications to use differne models, it is important that the
 *        application has a common API that it can use for DL inference and
 *        this class provides just that.
 */

/**
 * \brief Constant for TFLITE RT API
 * \ingroup group_dl_inferer
 */
#define DL_INFER_RTTYPE_TFLITE              "tflitert"

/**
 * \brief Constant for DLR RT API
 * \ingroup group_dl_inferer
 */
#define DL_INFER_RTTYPE_DLR                 "tvmdlr"

/**
 * \brief Constant for ONNX RT API
 * \ingroup group_dl_inferer
 */
#define DL_INFER_RTTYPE_ONNX                "onnxrt"

namespace ti::dl_inferer
{

    /* Forward declaration. */
    class DLInferer;

    /**
     * \brief DLR Interface context. This captures various attributes associated
     *        with the inputs and outputs of the model. The 'data' field may be
     *        undefined but all other parameters will have valid values, if the
     *        model defines them.
     *
     *        For TVM generated models, the 'name' field could be NULL for the
     *        output paramaters.
     *
     * \ingroup group_dl_inferer
     */
    class DlTensor
    {
        public:
            /** Name of the element. */
            const char             *name{nullptr};

            /** String representation of the element type. */
            const char             *typeName{nullptr};

            /** Unified type across APIs. */
            DlInferType             type;

            /** Total size in bytes of the input. This should be equal to
             *  numElem * sizeof(type).
             */
            int64_t                 size{};

            /** Total number of elements in the input. This should be equal
             *  to the product of all dimensions. The size of the type is
             *  not accounted in this.
             */
            int64_t                 numElem{};

            /** Element size in bytes. */
            int32_t                 elemSize{};

            /** Dimensions. */
            int32_t                 dim{};

            /** Shape information. */
            std::vector<int64_t>    shape;

            /** Data buffer. */
            void                   *data{nullptr};

            /**
             * Default constructor.
             */
            DlTensor();

            /**
             * Copy constructor.
             */
            DlTensor(const DlTensor& rhs);

            /**
            * Custom constructor.
            * Calculates required DlTensor fields based on tensorType and
            * tensorShape.
            */
            DlTensor(const DlInferType          tensorType,
                    const std::vector<int64_t>  tensorShape);
            /**
             * Dumps the information to the screen.
             */
            void dumpInfo() const;

            /**
             * Allocate memory for the buffer.
             */
            void allocateDataBuffer(DLInferer& inferer);

            /**
             * Assignment operator.
             *
             */
            DlTensor &operator=(const DlTensor& rhs);

            /**
             * Destructor
             */
            ~DlTensor();

        private:

            /** Flag to track if memory has been allocated. */
            bool        dataAllocated{false};
    };

    /**
     * \brief Alias for a vector of DL interface info objects.
     *
     * \ingroup group_dl_inferer
     */
    using VecDlTensor = std::vector<DlTensor>;

    /** Alias for a vector of DlTensor pointers
     * \ingroup group_dl_inferer
     */
    using VecDlTensorPtr = std::vector<DlTensor *>;

    /** \brief An abstract base class for different class of RT inference API.
     *
     * \ingroup group_dl_inferer
     */
    class DLInferer
    {
        public:
            /**
             * Runs the model.
             *
             * @param inputs Input buffers to set for inference run
             * @param outputs Output buffers to set for inference run
             *
             * @returns 0 upon success. A nagative value otherwise.
             */
            virtual int32_t run(const VecDlTensorPtr &inputs,
                                VecDlTensorPtr       &outputs) = 0;

            /**
             * Dumps the model information to the screen.
             */
            virtual void dumpInfo() = 0;

            /**
             * Returns an array containing detailed information on the inputs
             * of the model.
             *
             * @returns An array of input interface parameters.
             */
            virtual const VecDlTensor *getInputInfo() = 0;

            /**
             * Returns an array containing detailed information on the outputs
             * of the model.
             *
             * @returns An array of output interface parameters.
             */
            virtual const VecDlTensor *getOutputInfo() = 0;

            /**
             * Returns an allocated pointer that can be consumed by inference
             * of the model by this framework.
             *
             * @returns An pointer to allocated memory.
             */
            virtual void *allocate(int64_t size)
            {
                return new uint8_t [size];
            }

            /** Factory method for making a specifc inferer based on the
             * configuration passed.
             *
             * @param config Configuration specifying the type of inferer and
             *               associated parameters.
             * @returns A valid inferer if success. A nullptr otherwise.
             */
            static DLInferer* makeInferer(const InfererConfig &config);

            /**
             * Creates the descriptor based on the information from the
             * inference model interface information.
             *
             * @param ifInfoList Vector of inference model interface parameters
             * @param vecVar     Vector of descriptors created by this function
             * @param allocate   Allocate memory if True
             */
            int32_t createBuffers(const VecDlTensor    *ifInfoList,
                                  VecDlTensorPtr        &vecVar,
                                  bool                  allocate);

            /**
             * Destructor.
             */
            virtual ~DLInferer(){}

        protected:
            /** Mutex for multi-thread access control. */
            std::mutex  m_mutex;
    };

#define DL_INFER_GET_EXCL_ACCESS    std::unique_lock<std::mutex> lock(this->m_mutex)
} // namespace ti::dl_inferer

#endif // _TI_DL_INFERER_
