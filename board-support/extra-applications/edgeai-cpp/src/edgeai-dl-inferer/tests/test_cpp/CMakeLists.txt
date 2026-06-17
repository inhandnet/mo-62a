cmake_minimum_required(VERSION 3.10.0)

project(app_dl_inferer_test)

include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)

include_directories(${CMAKE_SOURCE_DIR})

set(SYSTEM_LINK_LIBS ${SYSTEM_LINK_LIBS}
    opencv_core
    opencv_imgproc
    opencv_imgcodecs)

set(SRC_FILES
    src/app_dl_inferer_cmd_line_parse.cpp
    src/app_dl_inferer_utils.cpp
    src/app_dl_inferer_inference.cpp
    src/app_dl_inferer_main.cpp)

build_app(${PROJECT_NAME}
          ${SRC_FILES})