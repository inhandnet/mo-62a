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
#if !defined(_TI_POST_PROCESS_CONFIG_)
#define _TI_POST_PROCESS_CONFIG_

/* Standard headers. */
#include <string>
#include <vector>
#include <map>

/* Module headers. */
#include <ti_dl_inferer.h>

/**
 * \defgroup group_post_process_config Post Process Helper Library
 *
 * \brief Unified interface for parsing model file to get post processing 
 *        configurations
 *
 */

 /**
 * \brief Constant for Default Post Proc Width
 * \ingroup group_post_process_config
 */
#define POSTPROC_DEFAULT_WIDTH   1280

/**
 * \brief Constant for Default Post Proc Height
 * \ingroup group_post_process_config
 */
#define POSTPROC_DEFAULT_HEIGHT  720

/**
 * \brief Constant for Default Post Proc Disp Width
 * \ingroup group_post_process_config
 */
#define DEFAULT_DISP_WIDTH       1920

/**
 * \brief Constant for Default Post Proc Disp Height
 * \ingroup group_post_process_config
 */
#define DEFAULT_DISP_HEIGHT      1080

namespace ti::post_process
{
    using namespace ti::dl_inferer;

    /**
     * \brief Struct to contain dataset information
     *
     * \ingroup group_post_process_config
     */
    struct DatasetInfo
    {
        /** ID */
        int32_t                                 id{-1};

        /** Supercategory */
        std::string                             superCategory{""};

        /** Class name */
        std::string                             name{""};

        /** RGB Color */
        std::vector<uint8_t>                    rgbColor{};

        /** YUV Color */
        std::vector<uint8_t>                    yuvColor{};

        /** Keypoints */
        std::vector<std::string>                keypoints{};

        /** Keypoint skeleton */
        std::vector<std::vector<int8_t>>        skeleton{};
    };

    /**
     * \brief Configuration for the Post Process.
     *
     * \ingroup group_post_process_config
     */
    struct PostprocessImageConfig
    {
        /** Name of the model. */
        std::string                             modelName{};

        /** Type of the runtime API to invoke. The valid values are:
         * - DL_INFER_RTTYPE_DLR
         * - DL_INFER_RTTYPE_TFLITE
         * - DL_INFER_RTTYPE_ONNX
         */
        std::string                             rtType{};

        /** Task type.
         *  - detection
         *  - segmentation
         *  - classification
         */
        std::string                             taskType{};

        /** Layout of the data. Allowed values. */
        std::string                             dataLayout{"NCHW"};

        /** Optional offset to be applied when detecting the output
         * class. This is applicable for image classification and
         * detection cases only.
         * Classification - a single scalar value
         * Detection      - a map
         */
        std::map<int32_t,int32_t>               labelOffsetMap{{0,0}};

        /** Order of results for detection use case
         * default is assumed to be [0 1 2 3 4 5] which means
         * [x1y1 x2y2 label score]
         */
        std::vector<int32_t>                    formatter{0, 1, 2, 3, 4, 5};

        /** Ignore given index*/
        int32_t                                 ignoreIndex{-1};

        /** If detections are normalized to 0-1 */
        bool                                    normDetect{false};

        /** Order of tensors for detection results */
        std::vector<int32_t>                    resultIndices{0, 1, 2, 3};

        /** Multiplicative factor to be applied to Y co-ordinates. This is used
         * for visualization of the bounding boxes for object detection post-
         * processing only.
         */
        float                                   vizThreshold{0.50f};

        /** Alpha value for blending. This is used for semantic segmentation
         *  post-processing only.
         */
        float                                   alpha{0.2f};

        /** Number of classification results to pick from the top of the model output. */
        int32_t                                 topN{5};

        /** Width of the output to display after adding tile. */
        int32_t                                 dispWidth{DEFAULT_DISP_WIDTH};

        /** Height of the output to display after adding tile. */
        int32_t                                 dispHeight{DEFAULT_DISP_HEIGHT};

        /** Width of the input data. */
        int32_t                                 inDataWidth{POSTPROC_DEFAULT_WIDTH};

        /** Height of the input data. */
        int32_t                                 inDataHeight{POSTPROC_DEFAULT_HEIGHT};

        /** Width of the output data. */
        int32_t                                 outDataWidth{POSTPROC_DEFAULT_WIDTH};

        /** Height of the output data. */
        int32_t                                 outDataHeight{POSTPROC_DEFAULT_HEIGHT};

        /** Map containing dataset information. */
        std::map<int32_t, DatasetInfo>          datasetInfo{};

        /** Data type of Output tensors from inferer. */
        std::vector<DlInferType>                outputTensorTypes{};

        /** Shape of Output tensors from inferer. */
        std::vector<std::vector<int64_t>>       outputTensorShapes{};

        /**
         * Helper function to dump the configuration information.
         */
        void dumpInfo();

        /** Helper function to parse post process configuration. */
        int32_t getConfig(const std::string &modelBasePath);

        /** Helper function to parse dataset.yaml and get info from it. */
        void    getDatasetInfo(const std::string &modelBasePath);
    };

} // namespace ti::post_process

#endif // _TI_POST_PROCESS_CONFIG_
