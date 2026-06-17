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
#include <ti_dl_inferer_config.h>
#include <ti_dl_inferer_logger.h>

using namespace std;
using namespace ti::dl_inferer::utils;

namespace ti::dl_inferer
{

DlInferType getDataType(const std::string &type)
{
    std::string dType = type;
    //Convert to lower case
    for (auto &c : dType)
    {
        c = tolower(c);
    }

    DlInferType tensorType = DlInferType_Invalid;
    if (dType.find("uint8") != string::npos)
        tensorType = DlInferType_UInt8;
    else if (dType.find("uint16") != string::npos)
        tensorType = DlInferType_UInt16;
    else if (dType.find("uint32") != string::npos)
        tensorType = DlInferType_UInt32;
    else if (dType.find("int8") != string::npos)
        tensorType = DlInferType_Int8;
    else if (dType.find("int16") != string::npos)
        tensorType = DlInferType_Int16;
    else if (dType.find("int32") != string::npos)
        tensorType = DlInferType_Int32;
    else if (dType.find("int") != string::npos)
        tensorType = DlInferType_Int64;
    else if (dType.find("float16") != string::npos)
        tensorType = DlInferType_Float16;
    else if (dType.find("float") != string::npos)
        tensorType = DlInferType_Float32;
    return tensorType;
}

uint8_t getTypeSize(DlInferType type)
{
    switch (type) {
        case DlInferType_Int8:
        case DlInferType_UInt8:
            return 1;
        case DlInferType_Int16:
        case DlInferType_UInt16:
        case DlInferType_Float16:
            return 2;
        case DlInferType_Int32:
        case DlInferType_UInt32:
        case DlInferType_Float32:
            return 4;
        case DlInferType_Int64:
            return 8;
        default:
            return 0;
    }
}

void InfererConfig::dumpInfo()
{
    DL_INFER_LOG_INFO("InfererConfig::Model Path        = %s\n", modelFile.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Artifacts Path    = %s\n", artifactsPath.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Runtime API       = %s\n", rtType.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Device Type       = %s\n", devType.c_str());
    DL_INFER_LOG_INFO("InfererConfig::Enable TIDL       = %d\n", enableTidl);
    DL_INFER_LOG_INFO("InfererConfig::Core Number       = %d\n", coreNumber);
    DL_INFER_LOG_INFO_RAW("\n");
}

int32_t InfererConfig::getConfig(const std::string  &modelBasePath,
                                 const bool          enableTidlDelegate,
                                 const int           coreNum
                                )
{
    const string        &paramFile = modelBasePath + "/param.yaml";
    int32_t             status = 0;

    if (!std::filesystem::exists(paramFile))
    {
        DL_INFER_LOG_ERROR("The file [%s] does not exist.\n",paramFile.c_str());
        status = -1;
        return status;
    }

    const YAML::Node    config = YAML::LoadFile(paramFile.c_str());;
    const YAML::Node    &n = config["session"];
    enableTidl = enableTidlDelegate;
    coreNumber = coreNum;


    /** Validate the parsed yaml configuration and create the configuration
    * for the inference object creation.
    */
    if (!n)
    {
        DL_INFER_LOG_ERROR("Inference configuration parameters  missing.\n");
        status = -1;
    }
    else if (!n["model_path"])
    {
        DL_INFER_LOG_ERROR("Please specifiy a valid model path.\n");
        status = -1;
    }
    else if (!n["artifacts_folder"])
    {
        DL_INFER_LOG_ERROR("Artifacts directory path missing.\n");
        status = -1;
    }
    else if (!n["session_name"])
    {
        DL_INFER_LOG_ERROR("Please specifiy a valid run-time API type.\n");
        status = -1;
    }
    else if (!n["input_details"])
    {
        DL_INFER_LOG_ERROR("Please specifiy valid input tensor details.\n");
        status = -1;
    }
    else if (!n["output_details"])
    {
        DL_INFER_LOG_ERROR("Please specifiy valid output tensor details.\n");
        status = -1;
    }
    else
    {
        /* Initialize the inference configuration parameter object. */
        rtType = n["session_name"].as<string>();

        if (n["model_path"])
        {
            const string &modSrc = n["model_path"].as<string>();
            std::filesystem::path modPath(modSrc);
            if (modPath.is_relative())
            {
                modelFile = modelBasePath + "/" + modSrc;
            }
            else
            {
                modelFile = modSrc;
            }
        }

        if (n["artifacts_folder"])
        {
            const string &artSrc = n["artifacts_folder"].as<string>();
            std::filesystem::path artPath(artSrc);
            if (artPath.is_relative())
            {
                artifactsPath = modelBasePath + "/" + artSrc;
            }
            else
            {
                artifactsPath = artSrc;
            }
        }
        else
        {
            artifactsPath = modelBasePath + "/artifacts";
        }

        if (n["device_type"])
        {
            devType = n["device_type"].as<string>();
        }
        else
        {
            devType = "CPU";
        }

        if (n["device_id"])
        {
            devId = n["device_id"].as<int32_t>();
        }
        else
        {
            devId = 0;
        }

        if (n["input_data_layout"])
        {
            dataLayout = n["input_data_layout"].as<std::string>();
        }

        // Read the Input Tensor Details to inferer
        const YAML::Node &inputDetailsNode = n["input_details"];
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

        // Read the Tensor Details for output from inferer
        const YAML::Node &outputDetailsNode = n["output_details"];
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

} // namespace ti::dl_inferer