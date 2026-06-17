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

#ifndef _APP_DL_INFERER_CMD_LINE_PARSE_H_
#define _APP_DL_INFERER_LINE_PARSE_H_

/* Standard headers. */
#include <string>
#include <ti_dl_inferer_logger.h>

namespace ti::app_dl_inferer::common
{
    class CmdlineArgs
    {
        public:
            void parse(int32_t argc, char *argv[]);

            /** Path to Model Directory. */
            std::string                 modelDirectory;

            /** Path to Test Image. */
            std::string                 testImage;
            
            float                       vizThreshold{0.6f};

            /** Alpha value for blending for semantic segmentation. */
            float                       alpha{0.2f};

            /** Number of classification results to pick from the top of the model output. */
            int32_t                     topN{5};

            /** Enable TIDL. */
            bool                        enableTidl{true};

            /** Log Level. */
            std::string                 logLevel{"1"};
    };

} // namespace ti::app_dl_inferer::common

#endif /* _APP_DL_INFERER_CMD_LINE_PARSE_H_ */
