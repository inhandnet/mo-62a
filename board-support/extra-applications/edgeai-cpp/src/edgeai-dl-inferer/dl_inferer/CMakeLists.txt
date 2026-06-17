cmake_minimum_required(VERSION 3.10.0)

project(edgeai_dl_inferer)

include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)

set(DL_INFERER_SRCS
    src/ti_dl_inferer.cpp
    src/ti_dl_inferer_config.cpp
    src/ti_dl_inferer_logger.cpp)

if(USE_DLR_RT)
    set(DL_INFERER_SRCS ${DL_INFERER_SRCS} src/ti_dlr_inferer.cpp)
endif()

if(USE_TENSORFLOW_RT)
    set(DL_INFERER_SRCS ${DL_INFERER_SRCS} src/ti_tflite_inferer.cpp)
endif()

if(USE_ONNX_RT)
    set(DL_INFERER_SRCS ${DL_INFERER_SRCS} src/ti_onnx_inferer.cpp)
endif()

build_lib(${PROJECT_NAME}   # Named argument: library name
          STATIC            # Named argument: library type
          1.0.0             # Named argument: library version
          # The following all will be clubbed under ${ARGN}
          ${DL_INFERER_SRCS})
