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
/* Standard headers. */
#include <string>
#include <filesystem>

/* Third-party headers. */
#include <yaml-cpp/yaml.h>

/* Module headers. */
#include <ti_pre_process_config.h>
#include <ti_dl_inferer_logger.h>

using namespace std;
using namespace ti::dl_inferer::utils;

namespace ti::pre_process
{

void PreprocessImageConfig::dumpInfo()
{
    DL_INFER_LOG_INFO_RAW("\n");
    DL_INFER_LOG_INFO("PreprocessImageConfig::modelName       = %s\n", modelName.c_str());
    DL_INFER_LOG_INFO("PreprocessImageConfig::rtType          = %s\n", rtType.c_str());
    DL_INFER_LOG_INFO("PreprocessImageConfig::taskType        = %s\n", taskType.c_str());
    DL_INFER_LOG_INFO("PreprocessImageConfig::dataLayout      = %s\n", dataLayout.c_str());
    DL_INFER_LOG_INFO("PreprocessImageConfig::inDataWidth     = %d\n", inDataWidth);
    DL_INFER_LOG_INFO("PreprocessImageConfig::inDataHeight    = %d\n", inDataHeight);
    DL_INFER_LOG_INFO("PreprocessImageConfig::resizeWidth     = %d\n", resizeWidth);
    DL_INFER_LOG_INFO("PreprocessImageConfig::resizeHeight    = %d\n", resizeHeight);
    DL_INFER_LOG_INFO("PreprocessImageConfig::outDataWidth    = %d\n", outDataWidth);
    DL_INFER_LOG_INFO("PreprocessImageConfig::outDataHeight   = %d\n", outDataHeight);

    DL_INFER_LOG_INFO("PreprocessImageConfig::mean          = [");
    for (uint32_t i = 0; i < mean.size(); i++)
    {
        DL_INFER_LOG_INFO_RAW(" %f", mean[i]);
    }

    DL_INFER_LOG_INFO_RAW(" ]\n");

    DL_INFER_LOG_INFO("PreprocessImageConfig::scale         = [");
    for (uint32_t i = 0; i < scale.size(); i++)
    {
        DL_INFER_LOG_INFO_RAW(" %f", scale[i]);
    }

    DL_INFER_LOG_INFO_RAW(" ]\n\n");
}


int32_t PreprocessImageConfig::getConfig(const std::string &modelBasePath)
{
    const string        &paramFile = modelBasePath + "/param.yaml";
    int32_t             status = 0;

    if (!std::filesystem::exists(paramFile))
    {
        DL_INFER_LOG_ERROR("The file [%s] does not exist.\n",paramFile.c_str());
        status = -1;
        return status;
    }

    string model = modelBasePath;
    if (model.back() == '/')
    {
        model.pop_back();
    }
    modelName = std::filesystem::path(model).filename();

    const YAML::Node    yaml = YAML::LoadFile(paramFile.c_str());

    const YAML::Node   &session = yaml["session"];
    const YAML::Node   &task = yaml["task_type"];
    const YAML::Node   &preProc = yaml["preprocess"];

    /** Validate the parsed yaml configuration and create the configuration
    * for the inference object creation
    */
    if (!preProc)
    {
        DL_INFER_LOG_ERROR("Preprocess configuration parameters missing.\n");
        status = -1;
    }
    else if (!session["input_mean"])
    {
        DL_INFER_LOG_ERROR("Mean value specification missing.\n");
        status = -1;
    }
    else if (!session["input_scale"])
    {
        DL_INFER_LOG_ERROR("Scale value specification missing.\n");
        status = -1;
    }
    else if (!session["input_details"])
    {
        DL_INFER_LOG_ERROR("Input tensor details missing.\n");
        status = -1;
    }
    else if (!preProc["data_layout"])
    {
        DL_INFER_LOG_ERROR("Data layout specification missing.\n");
        status = -1;
    }
    else if (!preProc["resize"])
    {
        DL_INFER_LOG_ERROR("Resize specification missing.\n");
        status = -1;
    }
    else if (!preProc["crop"])
    {
        DL_INFER_LOG_ERROR("Crop specification missing.\n");
        status = -1;
    }
    /* Check if crop information exists. */
    if (status == 0)
    {
        rtType   = session["session_name"].as<string>();
        taskType = task.as<string>();

        // Read the width and height values
        const YAML::Node &cropNode = preProc["crop"];

        if (cropNode.Type() == YAML::NodeType::Sequence)
        {
            outDataHeight = cropNode[0].as<int32_t>();
            outDataWidth  = cropNode[1].as<int32_t>();
        }
        else if (cropNode.Type() == YAML::NodeType::Scalar)
        {
            outDataHeight = cropNode.as<int32_t>();
            outDataWidth  = outDataHeight;
        }

        // Read the data layout
        dataLayout = preProc["data_layout"].as<std::string>();

        if(preProc["reverse_channels"])
        {
            reverseChannel = preProc["reverse_channels"].as<bool>();
        }

        // Read the mean values
        const YAML::Node &meanNode = session["input_mean"];
        if (!meanNode.IsNull())
        {
            for (uint32_t i = 0; i < meanNode.size(); i++)
            {
                mean.push_back(meanNode[i].as<float>());
            }
        }

        // Read the scale values
        const YAML::Node &scaleNode = session["input_scale"];
        if (!scaleNode.IsNull())
        {
            for (uint32_t i = 0; i < scaleNode.size(); i++)
            {
                scale.push_back(scaleNode[i].as<float>());
            }
        }

        // Read the width and height values
        const YAML::Node &resizeNode = preProc["resize"];

        if (resizeNode.Type() == YAML::NodeType::Sequence)
        {
            resizeHeight = resizeNode[0].as<int32_t>();
            resizeWidth  = resizeNode[1].as<int32_t>();
        }
        else if (resizeNode.Type() == YAML::NodeType::Scalar)
        {
            int32_t resize = resizeNode.as<int32_t>();

            if (resize != outDataHeight || resize != outDataWidth)
            {
                int32_t minVal = std::min(inDataHeight, inDataWidth);

                /* tiovxmultiscaler dosen't support odd resolutions */
                resizeHeight = (((inDataHeight * resize)/minVal) >> 1) << 1;
                resizeWidth  = (((inDataWidth * resize)/minVal) >> 1) << 1;
            }
            else
            {
                resizeHeight = resize;
                resizeWidth  = resize;
            }
        }

        if (mean.size() != scale.size())
        {
            DL_INFER_LOG_ERROR("The sizes of mean and scale vectors do not match.\n");
            status = -1;
        }

        // Read the Input Tensor Details
        const YAML::Node &inputDetailsNode = session["input_details"];
        for (uint32_t i = 0; i < inputDetailsNode.size(); i++)
        {
            string dType = inputDetailsNode[i]["type"].as<string>();
            DlInferType parsedType = getDataType(dType);

            inputTensorTypes.push_back(parsedType);

            vector<int64_t> tShape;
            const YAML::Node &tShapeNode = inputDetailsNode[i]["shape"];
            for (uint32_t j = 0; j < tShapeNode.size(); j++)
            {
                int n;
                string shapeValue =  tShapeNode[j].as<string>();
                if ((std::istringstream(shapeValue) >> n >> std::ws).eof())
                {
                    tShape.push_back(tShapeNode[j].as<int64_t>());
                }
                else
                {
                    DL_INFER_LOG_WARN("Input Shape in params file is not an integer");
                    tShape.push_back(-1);
                }
            }

            inputTensorShapes.push_back(tShape);
        }
    }
    return status;
}

} // namespace ti::pre_process
