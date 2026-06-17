cmake_minimum_required(VERSION 3.10.0)

project(dump_model_info)

include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)

include_directories(${CMAKE_SOURCE_DIR})

build_app(${PROJECT_NAME}
          dump_model_info/src/dump_model_info_main.cpp)
