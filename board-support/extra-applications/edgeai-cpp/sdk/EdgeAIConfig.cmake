# EdgeAIConfig.cmake — MO-62A C/C++ Edge AI SDK
#
# Provides a single imported target, EdgeAI::edgeai, that carries every include
# path and link library needed to build an Edge AI inference program on the
# MO-62A board (TI AM62A7, C7x DSP via TIDL). It wraps the edgeai-dl-inferer
# library, the prebuilt TFLite/ONNX runtimes, OpenCV, GStreamer and the TI
# tivision_apps stack, so customer projects need only:
#
#     find_package(EdgeAI REQUIRED)
#     add_executable(my_infer main.cpp)
#     target_link_libraries(my_infer PRIVATE EdgeAI::edgeai)
#
# Works for both headless inference and full camera->infer->HDMI pipelines.
#
# Installed by the MO-62A SDK to /usr/lib/cmake/EdgeAI/EdgeAIConfig.cmake.

cmake_minimum_required(VERSION 3.16)

if(TARGET EdgeAI::edgeai)
  return()
endif()

# --- SDK install locations (laid down by the MO-62A image) -------------------
set(EDGEAI_INCLUDE_DIR "/usr/include/edgeai"   CACHE PATH "EdgeAI SDK headers")
set(EDGEAI_LIB_DIR     "/usr/lib/edgeai"       CACHE PATH "EdgeAI + TFLite static archives")
set(EDGEAI_TI_LIB_DIR  "/opt/ti/edgeai/lib"    CACHE PATH "TI shared libs (onnxruntime, tivision_apps)")

if(NOT EXISTS "${EDGEAI_INCLUDE_DIR}/ti_dl_inferer.h")
  message(FATAL_ERROR
    "EdgeAI SDK headers not found at ${EDGEAI_INCLUDE_DIR}. "
    "Is the MO-62A C/C++ SDK installed on this image?")
endif()

# --- system dependencies (dev packages preinstalled in the base image) -------
find_package(PkgConfig REQUIRED)
pkg_check_modules(EDGEAI_OPENCV REQUIRED IMPORTED_TARGET opencv4)
pkg_check_modules(EDGEAI_GST    REQUIRED IMPORTED_TARGET gstreamer-1.0 gstreamer-app-1.0)
pkg_check_modules(EDGEAI_GLIB   REQUIRED IMPORTED_TARGET glib-2.0 gobject-2.0)

find_library(EDGEAI_YAMLCPP_LIB NAMES yaml-cpp REQUIRED)
find_library(EDGEAI_NCURSES_LIB NAMES ncurses  REQUIRED)
find_library(EDGEAI_TINFO_LIB   NAMES tinfo)   # optional; folded into ncurses on some distros

# --- TI runtimes: ONNX Runtime MUST be the TIDL-enabled build (1.15), not the
#     Debian onnxruntime; tivision_apps provides the C7x/VPAC offload. ---------
find_library(EDGEAI_ONNXRT_LIB   NAMES onnxruntime   PATHS "${EDGEAI_TI_LIB_DIR}" NO_DEFAULT_PATH REQUIRED)
find_library(EDGEAI_TIVISION_LIB NAMES tivision_apps PATHS "${EDGEAI_TI_LIB_DIR}" NO_DEFAULT_PATH REQUIRED)

# --- static archives: edgeai_dl_inferer/pre/post + TFLite and its deps (ruy,
#     abseil, flatbuffers, XNNPACK stub, ...). Circular refs across these
#     archives require a single --start-group/--end-group span. ---------------
file(GLOB EDGEAI_STATIC_LIBS "${EDGEAI_LIB_DIR}/lib*.a")
if(NOT EDGEAI_STATIC_LIBS)
  message(FATAL_ERROR "EdgeAI SDK static libraries not found in ${EDGEAI_LIB_DIR}")
endif()

# --- the imported target ------------------------------------------------------
add_library(EdgeAI::edgeai INTERFACE IMPORTED)

set_target_properties(EdgeAI::edgeai PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES
    "${EDGEAI_INCLUDE_DIR};${EDGEAI_INCLUDE_DIR}/onnxruntime;${EDGEAI_INCLUDE_DIR}/onnxruntime/core/session"
)

set_property(TARGET EdgeAI::edgeai PROPERTY INTERFACE_LINK_LIBRARIES
  # archives wrapped so the linker resolves their mutual references
  "-Wl,--start-group"
  ${EDGEAI_STATIC_LIBS}
  "-Wl,--end-group"
  # TI shared runtimes (full path, pinned to the TIDL build)
  "${EDGEAI_ONNXRT_LIB}"
  "${EDGEAI_TIVISION_LIB}"
  # system shared libs (include dirs come with the imported targets)
  PkgConfig::EDGEAI_OPENCV
  PkgConfig::EDGEAI_GST
  PkgConfig::EDGEAI_GLIB
  "${EDGEAI_YAMLCPP_LIB}"
  "${EDGEAI_NCURSES_LIB}"
  $<$<BOOL:${EDGEAI_TINFO_LIB}>:${EDGEAI_TINFO_LIB}>
  pthread
  ${CMAKE_DL_LIBS}
)

if(NOT EdgeAI_FIND_QUIETLY)
  message(STATUS "Found EdgeAI SDK: ${EDGEAI_INCLUDE_DIR} (${EDGEAI_LIB_DIR})")
endif()
