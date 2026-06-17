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
#if !defined(_TI_PRE_PROCESS_CONFIG_)
#define _TI_PRE_PROCESS_CONFIG_

/* Standard headers. */
#include <string>
#include <vector>

/* Module headers. */
#include <ti_dl_inferer.h>

using namespace std;

/**
 * \defgroup group_pre_process_config Pre Process Helper Library
 *
 * \brief Unified interface for parsing model file to get pre processing 
 *        configurations
 *
 */

 /**
 * \brief Constant for Default Pre Proc Width
 * \ingroup group_pre_process_config
 */
#define PREPROC_DEFAULT_WIDTH    320

/**
 * \brief Constant for Default Pre Proc Height
 * \ingroup group_pre_process_config
 */
#define PREPROC_DEFAULT_HEIGHT   240

namespace ti::pre_process
{
    using namespace ti::dl_inferer;
    /**
     * \brief Configuration for the Pre Process.
     *
     * \ingroup group_dl_inferer
     */
    struct PreprocessImageConfig
    {
        /** Name of the model. */
        string             modelName{};

        /** Type of the runtime API to invoke. The valid values are:
         * - DL_INFER_RTTYPE_DLR
         * - DL_INFER_RTTYPE_TFLITE
         * - DL_INFER_RTTYPE_ONNX
         */
        string             rtType{};

        /** Task type.
         *  - detection
         *  - segmentation
         *  - classification
         */
        string                  taskType{};

        /** Width of the input data. */
        int32_t                 inDataWidth{PREPROC_DEFAULT_WIDTH};

        /** Height of the input data. */
        int32_t                 inDataHeight{PREPROC_DEFAULT_HEIGHT};

        /** Out width. */
        int32_t                 outDataWidth{PREPROC_DEFAULT_WIDTH};

        /** Out height. */
        int32_t                 outDataHeight{PREPROC_DEFAULT_HEIGHT};

        /** Mean values to apply during normalization. */
        vector<float>           mean;

        /** Scale values to apply during normalization. */
        vector<float>           scale;

        /** Resize width. */
        int32_t                 resizeWidth{PREPROC_DEFAULT_WIDTH};

        /** Resize height. */
        int32_t                 resizeHeight{PREPROC_DEFAULT_HEIGHT};

        /** Layout of the data. Allowed values. */
        string                  dataLayout{"NCHW"};

        int32_t                 numChans{0};

        /** If preprocess is reverse channel. */
        bool                    reverseChannel{false};

        /** Data type of Input tensors to inferer. */
        vector<DlInferType>     inputTensorTypes{};

        /** Shape of Input tensors to inferer. */
        vector<vector<int64_t>> inputTensorShapes{};

        /**
         * Helper function to dump the configuration information.
         */
        void dumpInfo();

        /** Helper function to parse pre process configuration. */
        int32_t getConfig(const string      &modelBasePath);
    };

} // namespace ti::pre_process

#endif // _TI_PRE_PROCESS_CONFIG_
