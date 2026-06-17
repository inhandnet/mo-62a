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
#include <dlfcn.h>

/* Third-party headers. */
#include <tensorflow/lite/c/common.h>
#include <tensorflow/lite/c/c_api.h>
#include <tensorflow/lite/util.h>

/* Module headers. */
#include <ti_tflite_inferer.h>
#include <ti_dl_inferer_logger.h>

using namespace ti::dl_inferer::utils;

namespace ti::dl_inferer
{
typedef void (*ErrorHandler)(const char*);
typedef TfLiteDelegate* (*Create_delegate)(char**,
                                           char**,
                                           size_t,
                                           void (*report_error)(const char *));

DlInferType Tflite2TiInferType(TfLiteType type)
{
    DlInferType tiType = DlInferType_Invalid;

    switch (type)
    {
        case kTfLiteFloat32:
            tiType = DlInferType_Float32;
            break;

        case kTfLiteInt16:
            tiType = DlInferType_Int16;
            break;

        case kTfLiteInt32:
            tiType = DlInferType_Int32;
            break;

        case kTfLiteInt64:
            tiType = DlInferType_Int64;
            break;

        case kTfLiteUInt8:
            tiType = DlInferType_UInt8;
            break;

        case kTfLiteInt8:
            tiType = DlInferType_Int8;
            break;

        case kTfLiteFloat16:
            tiType = DlInferType_Float16;
            break;

        default:
            tiType = DlInferType_Invalid;
            break;
    }

    return tiType;
}

TFLiteInferer::TFLiteInferer(const InfererConfig &config):
    m_modelPath(config.modelFile),
    m_artifactPath(config.artifactsPath),
    m_enableTidl(config.enableTidl),
    m_coreNumber(config.coreNumber),
    m_debug_level(config.debug_level)
{
    Create_delegate createPlugin;
    const char      *keys[3];
    const char      *values[3];
    TfLiteDelegate  *dlgPtr;
    void            *lib;
    size_t          numOptions = 0;
    int32_t         status = 0;

    m_model = tflite::FlatBufferModel::BuildFromFile(m_modelPath.c_str());
    if (m_model == nullptr)
    {
        DL_INFER_LOG_ERROR("Model build failed.\n");
        status = -1;
    }

    if (status == 0)
    {
        tflite::InterpreterBuilder(*m_model, m_resolver)(&m_interpreter);
        if (m_interpreter == nullptr)
        {
            DL_INFER_LOG_ERROR("Inferer construction failed.\n");
            status = -1;
        }
    }

    if (m_enableTidl)
    {
        // Setup delegate
        if (status == 0)
        {
            lib = dlopen("/usr/lib/libtidl_tfl_delegate.so", RTLD_NOW);

            if (lib == NULL)
            {
                DL_INFER_LOG_ERROR("Opening TIDL delegate shared library "
                                "failed [/usr/lib/libtidl_tfl_delegate.so].\n");
                DL_INFER_LOG_ERROR("Error: %s\n", dlerror());
                status = -1;
            }
        }

        if (status == 0)
        {
            createPlugin =
                (Create_delegate)dlsym(lib, "tflite_plugin_create_delegate");

            if (createPlugin == NULL)
            {
                DL_INFER_LOG_ERROR("Symbol lookup in delegate shared library "
                                "failed.\n");
                status = -1;
            }
        }

        if (status == 0)
        {
            char coreNumber[5];
            char debug_level[5];
            sprintf(coreNumber, "%d", m_coreNumber);
            sprintf(debug_level, "%d", m_debug_level);
            // Currently, we program just one option
            keys[0]    = "artifacts_folder";
            values[0]  = m_artifactPath.c_str();
            numOptions++;
            keys[1]    = "core_number";
            values[1]  = coreNumber;
            numOptions++;
            keys[2]    = "debug_level";
            values[2]  = debug_level;
            numOptions++;

            // **keys, **values, num_options, error_handler
            dlgPtr = createPlugin((char **)keys, (char **)values, numOptions, NULL);

            m_interpreter->ModifyGraphWithDelegate(dlgPtr);
        }
    }

    if (status == 0)
    {
        // Allocate input and output tensors
        if (m_interpreter->AllocateTensors() != kTfLiteOk)
        {
            DL_INFER_LOG_ERROR("Tensor allocation failed.\n");
            status = -1;
        }
    }

    if (status == 0)
    {
        // Query the input information
        status = populateInputInfo();
    }

    // Query the output information
    if (status == 0)
    {
        status = populateOutputInfo();
    }

    if (status < 0)
    {
        throw std::runtime_error("TFLiteInferer object creation failed.");
    }

    DL_INFER_LOG_DEBUG("CONSTRUCTOR\n");
}

int32_t TFLiteInferer::populateInputInfo()
{
    const std::vector<int> inputs  = m_interpreter->inputs();

    m_numInputs = inputs.size();

    // Reserve the storage
    m_inputs.assign(m_numInputs, DlTensor());

    for (uint32_t i = 0; i < m_numInputs; i++)
    {
        DlTensor           *info;
        const TfLiteTensor *tensor;
        TfLiteType          type;

        info = &m_inputs[i];

        // Total size of the data in bytes
        tensor         = m_interpreter->input_tensor(i);
        type           = TfLiteTensorType(tensor);
        info->name     = TfLiteTensorName(tensor);
        info->size     = TfLiteTensorByteSize(tensor);
        info->dim      = TfLiteTensorNumDims(tensor);
        info->typeName = TfLiteTypeGetName(type);
        info->type     = Tflite2TiInferType(type);
        info->numElem  = 1;

        info->shape.assign(info->dim, 0);
        for (int32_t j = 0; j < info->dim; j++)
        {
            info->shape[j] = TfLiteTensorDim(tensor, j);
            info->numElem *= info->shape[j];
        }

        info->elemSize = info->size/info->numElem;

    } // for (uint32_t i = 0; i < m_numInputs; i++)

    return 0;
}

int32_t TFLiteInferer::populateOutputInfo()
{
    const std::vector<int> outputs  = m_interpreter->outputs();

    m_numOutputs = outputs.size();

    // Reserve the storage
    m_outputs.assign(m_numOutputs, DlTensor());

    for (uint32_t i = 0; i < m_numOutputs; i++)
    {
        DlTensor           *info;
        const TfLiteTensor *tensor;
        TfLiteType          type;

        info = &m_outputs[i];

        // Total size of the data in bytes
        tensor         = m_interpreter->output_tensor(i);
        type           = TfLiteTensorType(tensor);
        info->name     = TfLiteTensorName(tensor);
        info->size     = TfLiteTensorByteSize(tensor);
        info->dim      = TfLiteTensorNumDims(tensor);
        info->typeName = TfLiteTypeGetName(type);
        info->type     = Tflite2TiInferType(type);
        info->numElem  = 1;

        info->shape.assign(info->dim, 0);
        for (int32_t j = 0; j < info->dim; j++)
        {
            info->shape[j] = TfLiteTensorDim(tensor, j);
            info->numElem *= info->shape[j];
        }

        info->elemSize = info->size/info->numElem;

    } // for (uint32_t i = 0; i < m_numOutputs; i++)

    return 0;
}

int32_t TFLiteInferer::run(const VecDlTensorPtr  &inputs,
                           VecDlTensorPtr        &outputs)
{
    DL_INFER_GET_EXCL_ACCESS;
    TfLiteStatus    tfStatus;
    int32_t         status = 0;

    if (m_numInputs != inputs.size())
    {
        DL_INFER_LOG_ERROR("Number of inputs does not match.\n");
        status = -1;
    }
    else if (m_numOutputs != outputs.size())
    {
        DL_INFER_LOG_ERROR("Number of outputs does not match.\n");
        status = -1;
    }

    /* Set inputs and outputs (zero-copy). */
    if (status == 0)
    {
        for (uint32_t i = 0; i < m_numInputs; i++)
        {
            int tensor_idx = m_interpreter->inputs()[i];
            const TfLiteTensor *tensor = m_interpreter->input_tensor(i);
            m_interpreter->SetCustomAllocationForTensor(tensor_idx,
                    {inputs[i]->data, TfLiteTensorByteSize(tensor)});
        }
        for (uint32_t i = 0; i < m_numOutputs; i++)
        {
            int tensor_idx = m_interpreter->outputs()[i];
            const TfLiteTensor *tensor = m_interpreter->output_tensor(i);
            m_interpreter->SetCustomAllocationForTensor(tensor_idx,
                    {outputs[i]->data, TfLiteTensorByteSize(tensor)});
        }
    }

    /* Run the model. */
    if (status == 0)
    {
        tfStatus = m_interpreter->Invoke();

        if (tfStatus != kTfLiteOk)
        {
            DL_INFER_LOG_ERROR("Invoke() failed.\n");
            status = -1;
        }
    }

    return status;
}

void TFLiteInferer::dumpInfo()
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

const VecDlTensor *TFLiteInferer::getInputInfo()
{
    return &m_inputs;
}

const VecDlTensor *TFLiteInferer::getOutputInfo()
{
    return &m_outputs;
}

void *TFLiteInferer::allocate(int64_t size)
{
    auto align = [](int alignment, int64_t size) {
        return alignment * ((size + (alignment - 1)) / alignment);
    };
    return aligned_alloc(tflite::kDefaultTensorAlignment,
                align(tflite::kDefaultTensorAlignment, size));

}

TFLiteInferer::~TFLiteInferer()
{
    DL_INFER_LOG_DEBUG("DESTRUCTOR\n");
}

} // namespace ti::dl_inferer

