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
#include <stdio.h>
#include <cstring>
#include <string>
#include <algorithm>

/* Third-party headers. */
#include <core/providers/tidl/tidl_provider_factory.h>

/* Module headers. */
#include <ti_onnx_inferer.h>
#include <ti_dl_inferer_logger.h>

using namespace ti::dl_inferer::utils;

namespace ti::dl_inferer
{
#define ONNX_NUM_DEFAULT_ELEM   (200)

int32_t Onnx2TiInferType(ONNXTensorElementDataType  type,
                         const char               **typeName,
                         DlInferType               &tiType)
{
    int32_t size;

    switch (type)
    {
        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8:
            tiType    = DlInferType_Int8;
            *typeName = "int8";
            size      = sizeof(int8_t);
            break;

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8:
            tiType = DlInferType_UInt8;
            *typeName = "uint8";
            size      = sizeof(uint8_t);
            break;

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16:
            tiType    = DlInferType_Int16;
            *typeName = "int16";
            size      = sizeof(int16_t);
            break;

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16:
            tiType    = DlInferType_UInt16;
            *typeName = "uint16";
            size      = sizeof(uint16_t);
            break;

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32:
            tiType    = DlInferType_Int32;
            *typeName = "int32";
            size      = sizeof(int32_t);
            break;

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32:
            tiType    = DlInferType_UInt32;
            *typeName = "uint32";
            size      = sizeof(uint32_t);
            break;

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64:
            tiType    = DlInferType_Int64;
            *typeName = "int64";
            size      = sizeof(int64_t);
            break;

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16:
            tiType    = DlInferType_Float16;
            *typeName = "float16";
            size      = sizeof(float);
            break;

        case ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT:
            tiType    = DlInferType_Float32;
            *typeName = "float32";
            size      = sizeof(float);
            break;

        default:
            tiType    = DlInferType_Invalid;
            *typeName = "invalid";
            size      = 0;
    }

    return size;
}

int32_t Ti2OnnxInferType(DlInferType                type,
                         const char               **typeName,
                         ONNXTensorElementDataType  &onnxType)
{
    int32_t size;

    switch (type)
    {
        case DlInferType_Int8:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8;
            *typeName = "int8";
            size      = sizeof(int8_t);
            break;

        case DlInferType_UInt8:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8;
            *typeName = "uint8";
            size      = sizeof(uint8_t);
            break;

        case DlInferType_Int16:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16;
            *typeName = "int16";
            size      = sizeof(int16_t);
            break;

        case DlInferType_UInt16:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16;
            *typeName = "uint16";
            size      = sizeof(uint16_t);
            break;

        case DlInferType_Int32:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32;
            *typeName = "int32";
            size      = sizeof(int32_t);
            break;

        case DlInferType_UInt32:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32;
            *typeName = "uint32";
            size      = sizeof(uint32_t);
            break;

        case DlInferType_Int64:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64;
            *typeName = "int64";
            size      = sizeof(int64_t);
            break;

        case DlInferType_Float16:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16;
            *typeName = "float16";
            size      = sizeof(float);
            break;

        case DlInferType_Float32:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT;
            *typeName = "float32";
            size      = sizeof(float);
            break;

        default:
            onnxType  = ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED;
            *typeName = "invalid";
            size      = 0;
    }

    return size;
}

ORTInferer::ORTInferer(const InfererConfig &config):
    m_modelPath(config.modelFile),
    m_artifactPath(config.artifactsPath),
    m_enableTidl(config.enableTidl),
    m_coreNumber(config.coreNumber),
    m_debug_level(config.debug_level),
    m_outputTensorShapes(config.outputTensorShapes),
    m_outputTensorTypes(config.outputTensorTypes),
    m_allocateOutBuf(config.allocateOutBuf),
    m_env(ORT_LOGGING_LEVEL_ERROR, __FUNCTION__),
    m_memInfo(Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault))
{
    OrtStatus              *ortStatus;
    Ort::SessionOptions     sessionOpts;
    c_api_tidl_options      tidlOpts{};
    int32_t                 status;

    sessionOpts.SetGraphOptimizationLevel(
            GraphOptimizationLevel::ORT_ENABLE_EXTENDED);

    sessionOpts.SetLogSeverityLevel(3);

    if (m_enableTidl)
    {
        sessionOpts.SetIntraOpNumThreads(1);
        OrtStatus *def_status = OrtSessionsOptionsSetDefault_Tidl(&tidlOpts);
        strcpy(tidlOpts.artifacts_folder, m_artifactPath.c_str());
        tidlOpts.core_number = m_coreNumber;
        tidlOpts.debug_level = m_debug_level;
        ortStatus = OrtSessionOptionsAppendExecutionProvider_Tidl(sessionOpts,
                                                                &tidlOpts);
    }
    else
    {
        sessionOpts.SetIntraOpNumThreads(2);
        ortStatus = NULL;
    }

    if (ortStatus == NULL)
    {
        m_session = new Ort::Session(m_env, m_modelPath.c_str(), sessionOpts);

        // Query the input information
        status = populateInputInfo();
    }
    else
    {
        status = -1;
        throw std::runtime_error("ORTInferer setting session opts failed");
    }

    // Query the output information
    if (status == 0)
    {
        status = populateOutputInfo();
    }

    if (status < 0)
    {
        throw std::runtime_error("ORTInferer object creation failed.");
    }

    DL_INFER_LOG_DEBUG("CONSTRUCTOR\n");
}

int32_t ORTInferer::populateInputInfo()
{
    int32_t numInfo;
    int32_t status = 0;

    /* Query the number of inputs. */
    numInfo = m_session->GetInputCount();
    m_numInputs = numInfo;

    /* Reserve the storage. */
    m_inputs.assign(numInfo, DlTensor());
    m_inputTypes.assign(numInfo, ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED);
    m_inputNames.assign(numInfo, nullptr);

    for (int32_t i = 0; i < numInfo; i++)
    {
        DlTensor   *info;

        info = &m_inputs[i];

        /* Query input name. */
        auto inputName = m_session->GetInputNameAllocated(i, m_allocator);
        info->name = inputName.get();

        if (info->name == nullptr)
        {
            DL_INFER_LOG_ERROR("GetInputNameAllocated(%d) failed.\n", i);
            status = -1;
            break;
        }

        m_inputNames[i] = info->name;
        m_inputNamesPtr.push_back(std::move(inputName));

        auto typeInfo = m_session->GetInputTypeInfo(i);
        auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();

        /* Query input shape. */
        info->shape = tensorInfo.GetShape();

        /* Query input dimensions. */
        info->dim     = tensorInfo.GetDimensionsCount();
        info->numElem = tensorInfo.GetElementCount();

        /* Get the type, type name, and size. */
        m_inputTypes[i] = tensorInfo.GetElementType();

        info->elemSize = Onnx2TiInferType(m_inputTypes[i],
                                      &info->typeName,
                                      info->type);

        info->size = info->numElem * info->elemSize;

        if (info->size == 0)
        {
            DL_INFER_LOG_ERROR("Invalid data size(%d).\n", i);
            status = -1;
            break;
        }

    } // for (int32_t i = 0; i < numInfo; i++)

    return status;
}

int32_t ORTInferer::populateOutputInfo()
{
    int32_t numInfo;
    int32_t status = 0;

    /* Query the number of outputs. */
    numInfo = m_outputTensorShapes.size();
    m_numOutputs = numInfo;

    /* Reserve the storage. */
    m_outputs.assign(numInfo, DlTensor());
    m_outputTypes.assign(numInfo, ONNX_TENSOR_ELEMENT_DATA_TYPE_UNDEFINED);
    m_outputNames.assign(numInfo, nullptr);

    for (int32_t i = 0; i < numInfo; i++)
    {
        DlTensor   *info;

        info = &m_outputs[i];

        /* Query output name. */
        auto outputName = m_session->GetOutputNameAllocated(i, m_allocator);
        info->name = outputName.get();

        if (info->name == nullptr)
        {
            DL_INFER_LOG_ERROR("GetOutputNameAllocated(%d) failed.\n", i);
            status = -1;
            break;
        }

        m_outputNames[i] = info->name;
        m_outputNamesPtr.push_back(std::move(outputName));

        auto typeInfo = m_session->GetOutputTypeInfo(i);
        auto tensorInfo = typeInfo.GetTensorTypeAndShapeInfo();

        /* Query output shape. */
        info->shape = m_outputTensorShapes[i];

        /* Query output dimensions. */
        info->dim     = info->shape.size();

        for (int32_t j = 0; j < info->dim; j++)
        {
            if (-1 == info->shape[j])
            {
                info->shape[j] = ONNX_NUM_DEFAULT_ELEM;
            }
        }

        /* Query output num elements. */
        info->numElem = 1;

        for (int32_t j = 0; j < info->dim; j++)
        {
            info->numElem *= info->shape[j];
        }

        /* Get the type, type name, and size. */
        info->type = m_outputTensorTypes[i];

        info->elemSize   = Ti2OnnxInferType(info->type,
                                            &info->typeName,
                                            m_outputTypes[i]);

        info->size = info->numElem * info->elemSize;
    } // for (int32_t i = 0; i < numInfo; i++)

    return status;
}

int32_t ORTInferer::run(const VecDlTensorPtr &inputs,
                        VecDlTensorPtr       &outputs)
{
    DL_INFER_GET_EXCL_ACCESS;
    //auto null_outs = std::find_if(m_outputs.begin(), m_outputs.end(), [](auto t) { return t.size < 0; });
    //auto null_ins = std::find_if(m_inputs.begin(), m_inputs.end(), [](auto t) { return t.size < 0; });

    /*if(null_outs == m_outputs.end() && null_ins == m_inputs.end())
    {
        return run_zerocopy(inputs, outputs);
    }
    else*/
    {
        return run_memcopy(inputs, outputs);
    }

}

int32_t ORTInferer::run_memcopy(const VecDlTensorPtr &inputs,
                                VecDlTensorPtr       &outputs)
{
    std::vector<Ort::Value> inputValues;
    std::vector<Ort::Value> outputValues;
    const Ort::RunOptions  &runOpts = Ort::RunOptions();
    int32_t                 status = 0;

    for (uint32_t i = 0; i < m_numInputs; i++)
    {
        const DlTensor *info = inputs[i];
        Ort::Value v = Ort::Value::CreateTensor(m_memInfo,
                                                (void *)info->data,
                                                (size_t)info->size,
                                                info->shape.data(),
                                                info->shape.size(),
                                                m_inputTypes[i]);

        inputValues.push_back(std::move(v));
    }

    for (uint32_t i = 0; i < m_numOutputs; i++)
    {
        outputValues.emplace_back(nullptr);
    }

    outputValues = m_session->Run(runOpts,
                                  m_inputNames.data(),
                                  inputValues.data(),
                                  m_numInputs,
                                  m_outputNames.data(),
                                  m_numOutputs);

    /* Copy the output buffers. */
    for (uint32_t i = 0; i < m_numOutputs; i++)
    {
        DlTensor       *info = outputs[i];
        auto           &tensor = outputValues[i];
        void           *src = tensor.GetTensorMutableData<void>();
        const auto     &tsInfo = tensor.GetTensorTypeAndShapeInfo();
        int32_t         newSize;

        /* Update the output tensor object details. */
        info->shape   = tsInfo.GetShape();
        info->dim     = tsInfo.GetDimensionsCount();
        info->numElem = tsInfo.GetElementCount();
        newSize       = info->numElem * info->elemSize;

        /* Allocate new buffers if the new size is not the same as the old one.
         * This is typically the case for the detection models where the actual
         * tensor output dimensions are not known until one inference is run.
         */
        if (newSize != info->size)
        {
            DL_INFER_LOG_DEBUG("NEW_SIZE = %d OLD_SIZE = %d\n",
                               newSize, info->size);
            info->size = newSize;
            if (m_allocateOutBuf)
                info->allocateDataBuffer(*this);
        }
        if (info->data != NULL && src != NULL && info->size > 0)
            memcpy(info->data, src, info->size);
    }

    return status;
}

int32_t ORTInferer::run_zerocopy(const VecDlTensorPtr &inputs,
                                 VecDlTensorPtr       &outputs)
{
    Ort::IoBinding binding(*m_session);
    const Ort::RunOptions  &runOpts = Ort::RunOptions();
    int32_t                 status = 0;

    for (uint32_t i = 0; i < m_numInputs; i++)
    {
        const DlTensor *info = inputs[i];
        Ort::Value v = Ort::Value::CreateTensor(m_memInfo,
                                                (void *)info->data,
                                                (size_t)info->size,
                                                info->shape.data(),
                                                info->shape.size(),
                                                m_inputTypes[i]);
        binding.BindInput(info->name, v);
    }

    for (uint32_t i = 0; i < m_numOutputs; i++)
    {
        const DlTensor *info = outputs[i];
        Ort::Value v = Ort::Value::CreateTensor(m_memInfo,
                                                (void *)info->data,
                                                (size_t)info->size,
                                                info->shape.data(),
                                                info->shape.size(),
                                                m_outputTypes[i]);
        binding.BindOutput(info->name, v);
    }


    m_session->Run(runOpts, binding);

    return status;
}

void ORTInferer::dumpInfo()
{
    DL_INFER_LOG_INFO("Model Path        = %s\n", m_modelPath.c_str());
    DL_INFER_LOG_INFO("Number of Inputs  = %d\n", m_numInputs);

    for (uint32_t i = 0; i < m_numInputs; i++)
    {
        DlTensor   *info = &m_inputs[i];

        DL_INFER_LOG_INFO("INPUT [%d]: \n", i);
        info->dumpInfo();
    }

    DL_INFER_LOG_INFO("Number of Outputs  = %d\n", m_numOutputs);

    for (uint32_t i = 0; i < m_numOutputs; i++)
    {
        DlTensor   *info = &m_outputs[i];

        DL_INFER_LOG_INFO("OUTPUT [%d]: \n", i);
        info->dumpInfo();
    }

}

const VecDlTensor *ORTInferer::getInputInfo()
{
    return &m_inputs;
}

const VecDlTensor *ORTInferer::getOutputInfo()
{
    return &m_outputs;
}

ORTInferer::~ORTInferer()
{
    DL_INFER_LOG_DEBUG("DESTRUCTOR\n");

    delete m_session;
}

void *ORTInferer::allocate(int64_t size)
{
    void *mem = NULL;

    if(size > 0)
    {
        mem = new uint8_t[size];
    }

    return mem;
}

} // namespace ti::dl_inferer

