/*
 *  Copyright (C) 2022 Texas Instruments Incorporated - http://www.ti.com/
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Module headers. */
#include <test_cpp/include/app_dl_inferer_utils.h>
#include <test_cpp/include/app_dl_inferer_inference.h>

#define TI_EDGEAI_GET_TIME() chrono::system_clock::now()

#define TI_EDGEAI_GET_DIFF(_START, _END) \
chrono::duration_cast<chrono::milliseconds>(_END - _START).count()

namespace ti::app_dl_inferer::common
{
/* Alias for time point type */
using TimePoint = std::chrono::time_point<std::chrono::system_clock>;

template <typename InputT, typename OutputT>
int32_t normalize(InputT  *inData,
                  OutputT *outData,
                  PreprocessImageConfig &config)
{
    int32_t         status   = 0;
    float           mean[3]  = {0,0,0};
    float           scale[3] = {1,1,1};

    for (int32_t i = 0 ; i < 3; i++)
    {
        if (config.mean.size() > i)
        {
            mean[i] = config.mean[i];
        }
        if (config.scale.size() > i)
        {
            scale[i] = config.scale[i];
        }
    }

    //Input format is assumed to be HWC packed and assume n = 1
    if (config.dataLayout == "NCHW")
    {
        for (int h = 0; h < config.outDataHeight; h++)
        {
            for (int w = 0; w < config.outDataWidth; w++)
            {
                for (int c = 0; c < 3; c++)
                {
                    OutputT val = inData[h * config.outDataWidth * 3 + w * 3 + c];
                    val = (val - mean[c]) * scale[c];
                    outData[c * config.outDataWidth * config.outDataHeight + h * config.outDataWidth + w] = val;
                }
            }
        }
    }

    else if (config.dataLayout == "NHWC")
    {
        // 'inData' and 'outData' have the same data formats but different
        // data types and hence element widths.
        for (int h = 0; h < config.outDataHeight; h++)
        {
            for (int w = 0; w < config.outDataWidth; w++)
            {
                for (int c = 0; c < 3; c++)
                {
                    OutputT val = static_cast<OutputT>(*inData);
                    val = (val - mean[c]) * scale[c];
                    *outData++ = val;
                    inData++;
                }
            }
        }
    }
    else
    {
        status = -1;
    }
    return status;
}
uint32_t InferencePipe::m_instCnt = 0;

InferencePipe::InferencePipe(DLInferer                 *infererObj,
                             PostprocessImage          *postProcObj,
                             PreprocessImageConfig     &preProcConfig):
    m_inferer(infererObj),
    m_postProcObj(postProcObj),
    m_preProcCfg(preProcConfig)
{
    const DlTensor     *ifInfo;
    const VecDlTensor  *dlInfOutputs;
    int32_t             status;

    /* Set the instance Id. */
    m_instId = m_instCnt++;

    // Query the output information for setting up the output buffers
    dlInfOutputs = m_inferer->getOutputInfo();
    m_numOutputs = dlInfOutputs->size();

    status = m_inferer->createBuffers(dlInfOutputs, m_inferOutputBuff, true);

    if (status < 0)
    {
        DL_INFER_LOG_ERROR("createBuffers(m_inferOutputBuff) failed.\n");
    }

    if (status == 0)
    {
        const VecDlTensor  *dlInfInputs;

        /* Query the input information for setting up the output buffers for
         * the pre-processing stage.
         */
        dlInfInputs = m_inferer->getInputInfo();
        m_numInputs = dlInfInputs->size();

        status = m_inferer->createBuffers(dlInfInputs, m_inferInputBuff, true);

        if (status < 0)
        {
            DL_INFER_LOG_ERROR("createBuffers(m_inferInputBuff) failed.\n");
        }
    }

    if (status < 0)
    {
        throw runtime_error("InferencePipe object creation failed.");
    }
}

int32_t InferencePipe::getInstId()
{
    return m_instId;
}

int InferencePipe::runModel(void *inputBuff,void *originalBuff)
{
    TimePoint   start;
    TimePoint   end;
    float       diff;

    auto       *buff = m_inferInputBuff[0];
    int32_t     ret;
    int32_t     status;

    if (buff->type == DlInferType_Int8)
    {
        ret = normalize(reinterpret_cast<const uint8_t*>(inputBuff),
                        reinterpret_cast<int8_t*>(buff->data),
                        m_preProcCfg);
    }
    else if (buff->type == DlInferType_UInt8)
    {
        ret = normalize(reinterpret_cast<const uint8_t*>(inputBuff),
                        reinterpret_cast<uint8_t*>(buff->data),
                        m_preProcCfg);
    }
    else if (buff->type == DlInferType_Int16)
    {
        ret = normalize(reinterpret_cast<const uint8_t*>(inputBuff),
                        reinterpret_cast<int16_t*>(buff->data),
                        m_preProcCfg);
    }
    else if (buff->type == DlInferType_UInt16)
    {
        ret = normalize(reinterpret_cast<const uint8_t*>(inputBuff),
                        reinterpret_cast<uint16_t*>(buff->data),
                        m_preProcCfg);
    }
    else if (buff->type == DlInferType_Int32)
    {
        ret = normalize(reinterpret_cast<const uint8_t*>(inputBuff),
                        reinterpret_cast<int32_t*>(buff->data),
                        m_preProcCfg);
    }
    else if (buff->type == DlInferType_UInt32)
    {
        ret = normalize(reinterpret_cast<const uint8_t*>(inputBuff),
                        reinterpret_cast<uint32_t*>(buff->data),
                        m_preProcCfg);
    }
    else if (buff->type == DlInferType_Int64)
    {
        ret = normalize(reinterpret_cast<const uint8_t*>(inputBuff),
                        reinterpret_cast<int64_t*>(buff->data),
                        m_preProcCfg);
    }
    else if (buff->type == DlInferType_Float32)
    {
        ret = normalize(reinterpret_cast<const uint8_t*>(inputBuff),
                        reinterpret_cast<float*>(buff->data),
                        m_preProcCfg);
    }

    // Run the model
    start = TI_EDGEAI_GET_TIME();
    status = m_inferer->run(m_inferInputBuff, m_inferOutputBuff);
    end = TI_EDGEAI_GET_TIME();

    diff = TI_EDGEAI_GET_DIFF(start, end);

    if (status < 0)
    {
        throw runtime_error("Inference failed.\n");
    }

    printf("\n[STATS] Inference-%d took %.2fms\n" , m_instId, diff);
    
    start = TI_EDGEAI_GET_TIME();
    (*m_postProcObj)(originalBuff,m_inferOutputBuff);
    end = TI_EDGEAI_GET_TIME();

    diff = TI_EDGEAI_GET_DIFF(start, end);

    printf("[STATS] PostProcess-%d took %.2fms\n" , m_instId, diff);

    return status;
}

/** Destructor. */
InferencePipe::~InferencePipe()
{
    m_inferInputBuff.clear();
    m_inferOutputBuff.clear();
}

} // namespace ti::edgeai::common
