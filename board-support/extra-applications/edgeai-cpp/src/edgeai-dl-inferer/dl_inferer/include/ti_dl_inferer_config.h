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
#if !defined(_TI_DL_INFERER_CONFIG_)
#define _TI_DL_INFERER_CONFIG_

/* Standard headers. */
#include <stdint.h>
#include <string>
#include <vector>

/**
 * \defgroup group_dl_inferer_config DL Inferer Helper Library
 *
 * \brief Unified interface for parsing model file to get inferer
 *        configurations
 *
 */

 /**
 * \brief Constant for DLR invalid device Id.
 * \ingroup group_dl_inferer_config
 */
#define DLR_DEVID_INVALID                   (-1)

namespace ti::dl_inferer
{
    /**
     * \brief Enumeration for the different data types used for identifying
     *        the data types at the interface.
     *
     * \ingroup group_dl_inferer_config
     */
    typedef enum
    {
        /**  Invalid type. */
        DlInferType_Invalid = 0,

        /** Data type signed 8 bit integer. */
        DlInferType_Int8    = 2,

        /** Data type unsigned 8 bit integer. */
        DlInferType_UInt8   = 3,

        /** Data type signed 16 bit integer. */
        DlInferType_Int16   = 4,

        /** Data type unsigned 16 bit integer. */
        DlInferType_UInt16  = 5,

        /** Data type signed 32 bit integer. */
        DlInferType_Int32   = 6,

        /** Data type unsigned 32 bit integer. */
        DlInferType_UInt32  = 7,

        /** Data type signed 64 bit integer. */
        DlInferType_Int64   = 8,

        /** Data type 16 bit floating point. */
        DlInferType_Float16 = 9,

        /** Data type 32 bit floating point. */
        DlInferType_Float32 = 10,

     } DlInferType;

    /** Helper function to get DlInferType from type string. */
    DlInferType getDataType(const std::string &type);

    /** Helper function to get size of Dltensor data types in bytes. */
    uint8_t     getTypeSize(DlInferType type);

    /**
     * \brief Configuration for the DL inferer.
     *
     * \ingroup group_dl_inferer_config
     */
    struct InfererConfig
    {
        /** Path to the model directory or a file.
         *  - file for TFLITE and ONNX
         *  - directory for DLR
         **/
        std::string                         modelFile{};

        /** Path to the directory containing the model artifacts. This is only
         *  valid for TFLITE models and is not looked at for the other ones.
         */
        std::string                         artifactsPath{};

        /** Type of the runtime API to invoke. The valid values are:
         * - DL_INFER_RTTYPE_DLR
         * - DL_INFER_RTTYPE_TFLITE
         * - DL_INFER_RTTYPE_ONNX
         */
        std::string                         rtType{};

        /** Type of the device. This field is specific to the DLR API and
         * is not looked at for the other ones. Please refer to the DLR API
         * specification for valid values this field can take.
         */
        std::string                         devType{};

        /** Id of the device. This field is specific to the DLR API and
         * is not looked at for the other ones. Please refer to the DLR API
         * specification for valid values this field can take.
         */
        int32_t                             devId{DLR_DEVID_INVALID};

        /** Should TIDL be enabled. This field is specific to the DLR API and 
         * is not looked at for the other ones. Please refer to the DLR API
         * specification for valid values this field can take.
         */
        bool                                enableTidl{};

        /** Layout of the data. Allowed values. */
        std::string                         dataLayout{"NCHW"};

        /** Core Number to offload to
         */
        int                                 coreNumber{1};

        /** Debug level
         */
        int                                 debug_level{0};

        /** Allocate Output Buffers
        */
        bool                                allocateOutBuf{true};

        /** Data type of Input tensors to inferer. */
        std::vector<DlInferType>            inputTensorTypes;

        /** Shape of Input tensors to inferer. */
        std::vector<std::vector<int64_t>>   inputTensorShapes;

        /** Data type of Output tensors from inferer. */
        std::vector<DlInferType>            outputTensorTypes{};

        /** Shape of Output tensors from inferer. */
        std::vector<std::vector<int64_t>>   outputTensorShapes{};

        /**
         * Helper function to dump the configuration information.
         */
        void dumpInfo();

        /** Helper function to parse inference configuration. */
        int32_t getConfig(const std::string  &modelBasePath,
                          const bool          enableTidlDelegate,
                          const int           coreNum);
    };

} // namespace ti::dl_inferer

#endif // _TI_DL_INFERER_CONFIG_
