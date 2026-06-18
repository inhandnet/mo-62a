# app_edgeai — full camera -> inference -> HDMI demo for MO-62A.
#
# This is the C/C++ counterpart of the Python apps_python/app_edgeai.py and
# accepts the same YAML config schema (see ../configs/). It is provided as a
# starting point: copy this directory, modify the sources, rebuild.
#
# Build (on the board):
#     mkdir build && cd build && cmake .. && make
# Run (needs root for C7x/VPAC + DRM; stop the desktop first for kmssink):
#     sudo systemctl stop lightdm
#     sudo -E ./app_edgeai ../configs/object_detection_csi.yaml
#
# Unlike the upstream TI build, this CMakeLists does not need TARGET_FS or the
# TI cross-compile setup: it links the single EdgeAI::edgeai target installed by
# the MO-62A SDK, which carries every include path and library.
cmake_minimum_required(VERSION 3.16)
project(app_edgeai CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(EdgeAI REQUIRED)

# Demo sources: common pipeline + perf utils + the main entry point.
file(GLOB APP_SRCS
    common/src/*.cpp
    utils/src/*.cpp
    app_edgeai/src/*.cpp)

add_executable(app_edgeai ${APP_SRCS})

# Sources include headers as <common/include/...> and <utils/include/...>,
# i.e. relative to this directory.
target_include_directories(app_edgeai PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(app_edgeai PRIVATE SOC_AM62A)
target_link_libraries(app_edgeai PRIVATE EdgeAI::edgeai)
