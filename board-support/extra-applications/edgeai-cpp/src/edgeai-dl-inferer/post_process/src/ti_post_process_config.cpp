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
#include <ti_post_process_config.h>
#include <ti_dl_inferer_logger.h>

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)
#define RGB2Y(R, G, B) CLIP(( ( 66*(R) + 129*(G) + 25*(B) + 128) >>8 ) + 16)
#define RGB2U(R, G, B) CLIP(( ( -38*(R) - 74*(G) + 112*(B) + 128) >>8) + 128)
#define RGB2V(R, G, B) CLIP((( 112*(R) - 94*(G) - 18*(B) + 128) >> 8) + 128)

using namespace std;
using namespace ti::dl_inferer::utils;

namespace ti::post_process
{
void PostprocessImageConfig::dumpInfo()
{
    DL_INFER_LOG_INFO_RAW("\n");
    DL_INFER_LOG_INFO("PostprocessImageConfig::modelName      = %s\n", modelName.c_str());
    DL_INFER_LOG_INFO("PostprocessImageConfig::rtType         = %s\n", rtType.c_str());
    DL_INFER_LOG_INFO("PostprocessImageConfig::taskType       = %s\n", taskType.c_str());
    DL_INFER_LOG_INFO("PostprocessImageConfig::dataLayout     = %s\n", dataLayout.c_str());
    DL_INFER_LOG_INFO("PostprocessImageConfig::inDataWidth    = %d\n", inDataWidth);
    DL_INFER_LOG_INFO("PostprocessImageConfig::inDataHeight   = %d\n", inDataHeight);
    DL_INFER_LOG_INFO("PostprocessImageConfig::outDataWidth   = %d\n", outDataWidth);
    DL_INFER_LOG_INFO("PostprocessImageConfig::outDataHeight  = %d\n", outDataHeight);
    DL_INFER_LOG_INFO("PostprocessImageConfig::vizThreshold   = %f\n", vizThreshold);
    DL_INFER_LOG_INFO("PostprocessImageConfig::alpha          = %f\n", alpha);
    DL_INFER_LOG_INFO("PostprocessImageConfig::normDetect     = %d\n", normDetect);
    DL_INFER_LOG_INFO("PostprocessImageConfig::labelOffsetMap = [ ");

    for (const auto& [key, value] : labelOffsetMap)
    {
        DL_INFER_LOG_INFO_RAW("(%d, %d) ", key, value);
    }

    DL_INFER_LOG_INFO_RAW("]\n\n");

    DL_INFER_LOG_INFO("PostprocessImageConfig::formatter = [ ");

    for (uint32_t i = 0; i < formatter.size(); i++)
    {
        DL_INFER_LOG_INFO_RAW(" %d", formatter[i]);
    }

    DL_INFER_LOG_INFO_RAW("]\n\n");

    DL_INFER_LOG_INFO("PostprocessImageConfig::resultIndices = [ ");

    for (uint32_t i = 0; i < resultIndices.size(); i++)
    {
        DL_INFER_LOG_INFO_RAW(" %d", resultIndices[i]);
    }

    DL_INFER_LOG_INFO_RAW("]\n\n");
}

int32_t PostprocessImageConfig::getConfig(const std::string      &modelBasePath)
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
    const YAML::Node   &postProc = yaml["postprocess"];

    /** Validate the parsed yaml configuration and create the configuration
    * for the inference object creation
    */
    if (!session)
    {
        DL_INFER_LOG_WARN("Inference configuration parameters  missing.\n");
        status = -1;
    }
    else if (!session["output_details"])
    {
        DL_INFER_LOG_ERROR("Output tensor details missing.\n");
        status = -1;
    }
    else if (!postProc)
    {
        DL_INFER_LOG_WARN("Postprocess configuration parameters missing.\n");
        status = -1;
    }
    else if (!task)
    {
        DL_INFER_LOG_WARN("Tasktype configuration parameters missing.\n");
        status = -1;
    }

    if (status == 0)
    {
        rtType   = session["session_name"].as<string>();
        taskType = task.as<string>();

        // Read the data layout
        if (postProc["data_layout"])
        {
            dataLayout = postProc["data_layout"].as<std::string>();
        }

        if (postProc["formatter"] && postProc["formatter"]["src_indices"])
        {
            const YAML::Node &formatterNode = postProc["formatter"]["src_indices"];

            /* default is assumed to be [0 1 2 3 4 5] which means
             * [x1y1 x2y2 label score].
             *
             * CASE 1: Only 2 values are specified. These are assumed to
             *         be "label" and "score". Keep [0..3] same as the default
             *         values but overwrite [4,5] with these two values.
             *
             * CASE 2: Only 4 values are specified. These are assumed to
             *         be "x1y1" and "x2y2". Keep [4,5] same as the default
             *         values but overwrite [0..3] with these four values.
             *
             * CASE 3: All 6 values are specified. Overwrite the defaults.
             *
             */
            if (formatterNode.size() == 2)
            {
                formatter[4] = formatterNode[0].as<int32_t>();
                formatter[5] = formatterNode[1].as<int32_t>();
            }
            else if ((formatterNode.size() == 6) ||
                     (formatterNode.size() == 4))
            {
                for (uint8_t i = 0; i < formatterNode.size(); i++)
                {
                    formatter[i] = formatterNode[i].as<int32_t>();
                }
            }
            else
            {
                DL_INFER_LOG_ERROR("formatter specification incorrect.\n");
                status = -1;
            }
        }

        if (postProc["ignore_index"] && !postProc["ignore_index"].IsNull())
        {
            ignoreIndex = postProc["ignore_index"].as<int32_t>();
        }

        if (postProc["normalized_detections"])
        {
            normDetect = postProc["normalized_detections"].as<bool>();
        }

        if (postProc["shuffle_indices"])
        {
            const YAML::Node indicesNode = postProc["shuffle_indices"];

            for (uint8_t i = 0; i < indicesNode.size(); i++)
            {
                resultIndices[i] = indicesNode[i].as<int32_t>();
            }
        }

        const YAML::Node   &metric = yaml["metric"];

        if (metric && metric["label_offset_pred"])
        {
            // Read the width and height values
            const YAML::Node &offset = metric["label_offset_pred"];

            if (offset.Type() == YAML::NodeType::Scalar)
            {
                /* Use "0" key to store the value. */
                labelOffsetMap[0] = offset.as<int32_t>();
            }
            else if (offset.Type() == YAML::NodeType::Map)
            {
                for (const auto& it : offset)
                {
                    if (it.second.Type() == YAML::NodeType::Scalar)
                    {
                        labelOffsetMap[it.first.as<int32_t>()] =
                            it.second.as<int32_t>();
                    }
                }
            }
            else
            {
                DL_INFER_LOG_ERROR("label_offset_pred specification incorrect.\n");
                status = -1;
            }
        }

        // Parse dataset file
        getDatasetInfo(modelBasePath);

        // Read the Tensor Details for output from inferer
        const YAML::Node &outputDetailsNode = session["output_details"];
        for (uint32_t i = 0; i < outputDetailsNode.size(); i++)
        {
            string dType = outputDetailsNode[i]["type"].as<string>();
            DlInferType parsedType = getDataType(dType);

            outputTensorTypes.push_back(parsedType);

            vector<int64_t> tShape;
            const YAML::Node &tShapeNode = outputDetailsNode[i]["shape"];
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
                    DL_INFER_LOG_WARN("Output Shape in params file is not an integer");
                    tShape.push_back(-1);
                }
            }
            outputTensorShapes.push_back(tShape);
        }

    }

    return status;
}

void PostprocessImageConfig::getDatasetInfo(const std::string &modelBasePath)
{
    int32_t         count = 0;
    const string    &datasetFile = modelBasePath + "/dataset.yaml";

    if (!std::filesystem::exists(datasetFile))
    {
        DL_INFER_LOG_WARN("The file [%s] does not exist.\n",datasetFile.c_str());
        return;
    }

    const YAML::Node        yaml = YAML::LoadFile(datasetFile.c_str());
    const YAML::Node        &categories = yaml["categories"];
    const YAML::Node        &color_map = yaml["color_map"];
    YAML::const_iterator    color_map_it;

    // Validate the parsed yaml configuration
    if (!categories)
    {
        DL_INFER_LOG_WARN("Parameter categories missing in dataset file.\n");
        return;
    }

    if(color_map)
    {
        color_map_it = color_map.begin();
    }

    for (YAML::Node data : categories)
    {
        DatasetInfo dInfo;

        if (data["id"])
        {
            dInfo.id = data["id"].as<int32_t>();
        }

        if (data["name"])
        {
            dInfo.name = data["name"].as<std::string>();
        }

        if (data["supercategory"])
        {
            dInfo.superCategory = data["supercategory"].as<std::string>();
        }

        if (data["supercategory"])
        {
            dInfo.superCategory = data["supercategory"].as<std::string>();
        }

        if (data["keypoints"])
        {
            dInfo.keypoints = data["keypoints"].as<std::vector<std::string>>();
        }

        if (data["skeleton"])
        {
            dInfo.skeleton = data["skeleton"].as<std::vector<std::vector<int8_t>>>();
        }

        dInfo.rgbColor.push_back(0);
        dInfo.rgbColor.push_back(255);
        dInfo.rgbColor.push_back(0);

        dInfo.yuvColor.push_back(149);
        dInfo.yuvColor.push_back(43);
        dInfo.yuvColor.push_back(21);

        if(color_map)
        {
            if(color_map_it != color_map.end())
            {
                const YAML::Node &color = *color_map_it;
                if(color)
                {
                    dInfo.rgbColor = color.as<std::vector<uint8_t>>();

                    uint8_t Y = RGB2Y(dInfo.rgbColor[0],
                                      dInfo.rgbColor[1],
                                      dInfo.rgbColor[2]);
                    uint8_t U = RGB2U(dInfo.rgbColor[0],
                                      dInfo.rgbColor[1],
                                      dInfo.rgbColor[2]);
                    uint8_t V = RGB2V(dInfo.rgbColor[0],
                                      dInfo.rgbColor[1],
                                      dInfo.rgbColor[2]);

                    dInfo.yuvColor.clear();
                    dInfo.yuvColor.push_back(Y);
                    dInfo.yuvColor.push_back(U);
                    dInfo.yuvColor.push_back(V);
                }
                color_map_it++;
            }
        }

        datasetInfo[dInfo.id] = dInfo;
        count++;
    }
}

} // namespace ti::post_process