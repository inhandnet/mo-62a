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

/* Module headers. */
#include <ti_dl_inferer.h>
#include <ti_dl_inferer_logger.h>

#if defined(USE_DLR_RT)
#include <ti_dlr_inferer.h>
#endif

#if defined(USE_TENSORFLOW_RT)
#include <ti_tflite_inferer.h>
#endif

#if defined(USE_ONNX_RT)
#include <ti_onnx_inferer.h>
#endif

/**
 * \brief Constant for DLR invalid device Id.
 * \ingroup group_dl_inferer
 */
#define DLR_DEVTYPE_INVALID                 (-1)

/**
 * \brief Constant for DLR device type CPU.
 * \ingroup group_dl_inferer
 */
#define DLR_DEVTYPE_CPU                     (1)

/**
 * \brief Constant for DLR device type GPU.
 * \ingroup group_dl_inferer
 */
#define DLR_DEVTYPE_GPU                     (2)

/**
 * \brief Constant for DLR device type OPENCL.
 * \ingroup group_dl_inferer
 */
#define DLR_DEVTYPE_OPENCL                  (3)

using namespace std;
using namespace ti::dl_inferer::utils;

namespace ti::dl_inferer
{

DlTensor::DlTensor()
{
    DL_INFER_LOG_DEBUG("DEFAULT CONSTRUCTOR\n");
}

DlTensor::DlTensor(const DlTensor& rhs):
    name(rhs.name),
    typeName(rhs.typeName),
    type(rhs.type),
    size(rhs.size),
    numElem (rhs.numElem),
    elemSize(rhs.elemSize),
    dim(rhs.dim),
    shape(rhs.shape),
    data(nullptr),
    dataAllocated(rhs.dataAllocated)
{
    /* Do not point to the other object's data pointer. It
     * should be allocated as needed in the current object.
     */
    DL_INFER_LOG_DEBUG("COPY CONSTRUCTOR\n");
}

DlTensor::DlTensor(const DlInferType        tensorType,
                   const vector<int64_t>    tensorShape):
    type(tensorType),
    shape(tensorShape),
    data(nullptr)
{
    DL_INFER_LOG_DEBUG("CUSTOM CONSTRUCTOR\n");

    elemSize = getTypeSize(type);
    dim = shape.size();

    numElem = 1;
    for (int32_t i = 0; i < dim; i++)
    {
        numElem *= shape[i];
    }

    size = numElem * elemSize;
}

void DlTensor::allocateDataBuffer(DLInferer& inferer)
{
    if (dataAllocated)
    {
        delete [] reinterpret_cast<uint8_t*>(data);
    }

    data = inferer.allocate(size);
    dataAllocated = true;
}

void DlTensor::dumpInfo() const
{
    if (name != nullptr)
    {
        DL_INFER_LOG_INFO("    Name          = %s\n", name);
    }

    DL_INFER_LOG_INFO("    Num Elements  = %ld\n", numElem);
    DL_INFER_LOG_INFO("    Element Size  = %d bytes\n", elemSize);
    DL_INFER_LOG_INFO("    Total Size    = %ld bytes\n", size);
    DL_INFER_LOG_INFO("    Num Dims      = %d\n", dim);
    DL_INFER_LOG_INFO("    Type          = %s (Enum: %d)\n", typeName, type);
    DL_INFER_LOG_INFO("    Shape         = ");

    for (int32_t j = 0; j < dim; j++)
    {
        DL_INFER_LOG_INFO_RAW("[%ld] ", shape[j]);
    }

    DL_INFER_LOG_INFO_RAW("\n\n");
}

DlTensor &DlTensor::operator=(const DlTensor& rhs)
{
    if (this != &rhs)
    {
        name     = rhs.name;
        typeName = rhs.typeName;
        type     = rhs.type;
        size     = rhs.size;
        elemSize = rhs.elemSize;
        numElem  = rhs.numElem;
        dim      = rhs.dim;
        shape    = rhs.shape;
        data     = nullptr;
    }

    return *this;
}

DlTensor::~DlTensor()
{
    DL_INFER_LOG_DEBUG("DESTRUCTOR\n");

    /* The allocation is based on the size in bytes andof type
     * uint8_t * to be generic. Given this, the following should
     * be safe to do so.
     */
    if (dataAllocated)
    {
        delete [] reinterpret_cast<uint8_t*>(data);
    }
}

DLInferer* DLInferer::makeInferer(const InfererConfig &config)
{
    DLInferer  *inter = nullptr;
    int32_t     status = 0;

    if (config.rtType.empty())
    {
        DL_INFER_LOG_ERROR("Please specifiy a valid run-time API type.\n");
        status = -1;
    }
#if defined(USE_TENSORFLOW_RT)
    else if (config.rtType == DL_INFER_RTTYPE_TFLITE)
    {
        if (config.artifactsPath.empty())
        {
            DL_INFER_LOG_ERROR("Missing model artifacts path.\n");
            status = -1;
        }
        else if (config.modelFile.empty())
        {
            DL_INFER_LOG_ERROR("Please specifiy a valid model path.\n");
            status = -1;
        }
        else
        {
            inter = new TFLiteInferer(config);
        }
    }
#endif

#if defined(USE_DLR_RT)
    else if (config.rtType == DL_INFER_RTTYPE_DLR)
    {
        const std::string  &s = config.devType;
        int32_t             devType = DLR_DEVTYPE_INVALID;

        if (config.devId < 0)
        {
            DL_INFER_LOG_ERROR("Missing device Id.\n");
            status = -1;
        }
        else if (s.empty())
        {
            DL_INFER_LOG_ERROR("Missing device type.\n");
            status = -1;
        }
        else if (s == "CPU")
        {
            devType = DLR_DEVTYPE_CPU;
        }
        else if (s == "GPU")
        {
            devType = DLR_DEVTYPE_GPU;
        }
        else if (s == "OPENCL")
        {
            devType = DLR_DEVTYPE_OPENCL;
        }

        if (status == 0)
        {
            inter = new DLRInferer(config, devType);
        }
    }
#endif

#if defined(USE_ONNX_RT)
    else if (config.rtType == DL_INFER_RTTYPE_ONNX)
    {
        if (config.artifactsPath.empty())
        {
            DL_INFER_LOG_ERROR("Missing model artifacts path.\n");
            status = -1;
        }
        else if (config.modelFile.empty())
        {
            DL_INFER_LOG_ERROR("Please specifiy a valid model path.\n");
            status = -1;
        }
        else
        {
            inter = new ORTInferer(config);
        }
    }
#endif
    else
    {
        DL_INFER_LOG_ERROR("Unsupported RT API.\n");
    }

    return inter;
}

int32_t DLInferer::createBuffers(const VecDlTensor    *ifInfoList,
                                 VecDlTensorPtr       &vecVar,
                                 bool                 allocate)
{
    vecVar.reserve(ifInfoList->size());

    for (uint64_t i = 0; i < ifInfoList->size(); i++)
    {
        const DlTensor *ifInfo = &ifInfoList->at(i);
        DlTensor   *obj = new DlTensor(*ifInfo);

        /* Allocate data buffer. */
        if (allocate)
            obj->allocateDataBuffer(*this);

        vecVar.push_back(obj);
    }

    return 0;
}

} // namespace ti::dl_inferer
