cmake_minimum_required(VERSION 3.10.0)

project(edgeai_pre_process)

include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)

set(PRE_PROCESS_SRCS
    src/ti_pre_process_config.cpp
    )


build_lib(${PROJECT_NAME}    # Named argument: library name
          STATIC              # Named argument: library type
          1.0.0               # Named argument: library version
          # The following all will be clubbed under ${ARGN}
          ${PRE_PROCESS_SRCS})
