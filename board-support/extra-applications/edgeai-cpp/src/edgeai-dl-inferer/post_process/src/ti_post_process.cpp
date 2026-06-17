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

/* Standard headers. */
#include <stdio.h>
#include <stdlib.h>

/* Module headers. */
#include <ti_post_process.h>
#include <ti_post_process_image_classification.h>
#include <ti_post_process_object_detection.h>
#include <ti_post_process_semantic_segmentation.h>
#include <ti_post_process_keypoint_detection.h>
#include <ti_dl_inferer_logger.h>

namespace ti::post_process
{

using namespace ti::dl_inferer::utils;

PostprocessImage::PostprocessImage(const PostprocessImageConfig &config):
    m_config(config)
{
    m_title = std::string("Model: ") + m_config.modelName;
}

PostprocessImage* PostprocessImage::makePostprocessImageObj(const PostprocessImageConfig    &config)
{
    PostprocessImage   *cntxt = nullptr;
    
    if (config.taskType == "classification")
    {
        cntxt = new PostprocessImageClassification(config);
    }
    else if (config.taskType == "detection")
    {
        cntxt = new PostprocessObjectDetection(config);
    }
    else if (config.taskType == "segmentation")
    {
        cntxt = new PostprocessSemanticSegmentation(config);
    }
    else if (config.taskType == "keypoint_detection")
    {
        cntxt = new PostprocessKeypointDetection(config);
    }
    else
    {
        DL_INFER_LOG_ERROR("Invalid post-processing task type.\n");
    }

    return cntxt;
}

const std::string &PostprocessImage::getTaskType()
{
    return m_config.taskType;
}

PostprocessImage::~PostprocessImage()
{
}

} // namespace  ti::post_process
