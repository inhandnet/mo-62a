// Copyright 2019 JD.com Inc. JD AI

#include "onnxruntime_c_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  int debug_level;
  char artifacts_folder[512];
  int priority;
  float max_pre_empt_delay;
  /* C7x core number to be used for inference */
  int core_number;
} c_api_tidl_options;

ORT_API_STATUS(OrtSessionsOptionsSetDefault_Tidl, _In_ c_api_tidl_options * tidl_options);
ORT_API_STATUS(OrtSessionOptionsAppendExecutionProvider_Tidl, _In_ OrtSessionOptions* options, c_api_tidl_options * tidl_options);

#ifdef __cplusplus
}
#endif


