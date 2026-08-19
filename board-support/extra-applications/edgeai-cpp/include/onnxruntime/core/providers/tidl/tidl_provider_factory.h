// Copyright 2019 JD.com Inc. JD AI

#include "onnxruntime_c_api.h"

#define TIDL_MAX_STRING_LENGTH (512)
#define TIDL_MAX_SUBGRAPH_DATA (128)
#define TIDL_MAX_OPTIONS (24)

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  char key[TIDL_MAX_STRING_LENGTH];
  char value[TIDL_MAX_STRING_LENGTH];
} c_api_tidl_option_pair;

typedef struct
{
  c_api_tidl_option_pair option[TIDL_MAX_OPTIONS];
  int count;
} c_api_tidl_options;

typedef struct
{
  uint64_t run_start;
  uint64_t run_end;
  uint64_t ddr_read_start;
  uint64_t ddr_read_end;
  uint64_t ddr_write_start;
  uint64_t ddr_write_end;
  uint64_t copy_in_start[TIDL_MAX_SUBGRAPH_DATA];
  uint64_t copy_in_end[TIDL_MAX_SUBGRAPH_DATA];
  uint64_t proc_start[TIDL_MAX_SUBGRAPH_DATA];
  uint64_t proc_end[TIDL_MAX_SUBGRAPH_DATA];
  uint64_t copy_out_start[TIDL_MAX_SUBGRAPH_DATA];
  uint64_t copy_out_end[TIDL_MAX_SUBGRAPH_DATA];
  uint32_t num_subgraph_data;
} c_api_tidl_benchmark_data;

// Initialize the TIDL options structure
ORT_API_STATUS(OrtSessionOptionsInitialize_Tidl, _Inout_ c_api_tidl_options* options);

// Set an option in the TIDL options
ORT_API_STATUS(OrtSessionOptionsSet_Tidl, _Inout_ c_api_tidl_options* options, _In_ const char* key, _In_ const char* value);

// Get an option from the TIDL options
ORT_API_STATUS(OrtSessionOptionsGet_Tidl, _In_ const c_api_tidl_options* options, _In_ const char* key,
               _Out_writes_bytes_all_(value_len) char* value, _In_ size_t value_len);

// Append TIDL execution provider with options
ORT_API_STATUS(OrtSessionOptionsAppendExecutionProvider_Tidl, _In_ OrtSessionOptions* session_options, _In_ const c_api_tidl_options* tidl_options);

// Get TIDL Performance data
ORT_API_STATUS(OrtSessionGetTIBenchmarkData_Tidl, _In_ OrtSession* session, _Out_ c_api_tidl_benchmark_data* benchmark_data);

#ifdef __cplusplus
}
#endif
