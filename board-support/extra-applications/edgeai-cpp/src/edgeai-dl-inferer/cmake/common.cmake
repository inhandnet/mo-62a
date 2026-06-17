include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_compile_options(-std=c++17 -fPIC)

option(USE_DLR_RT                    "Enable DLR inference" ON)
option(USE_TENSORFLOW_RT             "Enable Tensorflow inference" ON)
option(USE_ONNX_RT                   "Enable Onnx inference" ON)

# Specific compile optios across all targets
#add_compile_definitions(MINIMAL_LOGGING)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
ENDIF()


message(STATUS "CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE} PROJECT_NAME = ${PROJECT_NAME}")

SET(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib" ".so")

if(NOT CMAKE_OUTPUT_DIR)
    set(CMAKE_OUTPUT_DIR ${CMAKE_SOURCE_DIR})
endif()
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_OUTPUT_DIR}/bin/${CMAKE_BUILD_TYPE})
set(CMAKE_INSTALL_LIBDIR           lib)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr CACHE PATH "Installation Prefix" FORCE)
endif()

if (NOT DEFINED ENV{SOC})
    message(FATAL_ERROR "SOC not defined.")
endif()

set(TARGET_SOC_LOWER $ENV{SOC})

if ("${TARGET_SOC_LOWER}" STREQUAL "j721e")
    set(TARGET_PLATFORM     J7)
    set(TARGET_CPU          A72)
    set(TARGET_OS           LINUX)
    set(TARGET_SOC          J721E)
elseif ("${TARGET_SOC_LOWER}" STREQUAL "j721s2")
    set(TARGET_PLATFORM     J7)
    set(TARGET_CPU          A72)
    set(TARGET_OS           LINUX)
    set(TARGET_SOC          J721S2)
elseif ("${TARGET_SOC_LOWER}" STREQUAL "j784s4")
    set(TARGET_PLATFORM     J7)
    set(TARGET_CPU          A72)
    set(TARGET_OS           LINUX)
    set(TARGET_SOC          J784S4)
elseif ("${TARGET_SOC_LOWER}" STREQUAL "j722s")
    set(TARGET_PLATFORM     J7)
    set(TARGET_CPU          A53)
    set(TARGET_OS           LINUX)
    set(TARGET_SOC          J722S)
elseif ("${TARGET_SOC_LOWER}" STREQUAL "am62a")
    set(TARGET_PLATFORM     SITARA)
    set(TARGET_CPU          A53)
    set(TARGET_OS           LINUX)
    set(TARGET_SOC          AM62A)
elseif ("${TARGET_SOC_LOWER}" STREQUAL "am62x")
    set(TARGET_PLATFORM     SITARA)
    set(TARGET_CPU          A53)
    set(TARGET_OS           LINUX)
    set(TARGET_SOC          AM62X)
elseif ("${TARGET_SOC_LOWER}" STREQUAL "am62p")
    set(TARGET_PLATFORM     SITARA)
    set(TARGET_CPU          A53)
    set(TARGET_OS           LINUX)
    set(TARGET_SOC          AM62P)
else()
    message(FATAL_ERROR "SOC ${TARGET_SOC_LOWER} is not supported.")
endif()

message("SOC=${TARGET_SOC_LOWER}")

set(TARGET_PLATFORM     J7)
set(TARGET_CPU          A72)
set(TARGET_OS           LINUX)

set(TENSORFLOW_INSTALL_DIR ${TARGET_FS}/usr/include/tensorflow)
set(ONNXRT_INSTALL_DIR ${TARGET_FS}/usr/include/onnxruntime)
set(TFLITE_INSTALL_DIR ${TARGET_FS}/usr/lib/tflite_2.12)

add_definitions(
    -DTARGET_CPU=${TARGET_CPU}
    -DTARGET_OS=${TARGET_OS}
)

if(USE_DLR_RT)
add_definitions(-DUSE_DLR_RT)
endif()

if(USE_TENSORFLOW_RT)
add_definitions(-DUSE_TENSORFLOW_RT)
endif()

if(USE_ONNX_RT)
add_definitions(-DUSE_ONNX_RT)
endif()

link_directories(${TARGET_FS}/usr/lib/aarch64-linux-gnu
                 ${TARGET_FS}/usr/lib/
                 )

if(USE_DLR_RT)
link_directories(${TARGET_FS}/usr//local/dlr)
link_directories(${TARGET_FS}/usr/lib/python3.12/site-packages/dlr)
endif()

if(USE_TENSORFLOW_RT)
link_directories(${TENSORFLOW_INSTALL_DIR}/tensorflow/lite/tools/make/gen/linux_aarch64/lib
                 ${TFLITE_INSTALL_DIR}/ruy-build
                 ${TFLITE_INSTALL_DIR}/xnnpack-build
                 ${TFLITE_INSTALL_DIR}/pthreadpool
                 ${TFLITE_INSTALL_DIR}/fft2d-build
                 ${TFLITE_INSTALL_DIR}/cpuinfo-build
                 ${TFLITE_INSTALL_DIR}/flatbuffers-build
                 ${TFLITE_INSTALL_DIR}/abseil-cpp-build
                 ${TFLITE_INSTALL_DIR}/farmhash-build
)
endif()

if(USE_ONNX_RT)
endif()

#message("PROJECT_SOURCE_DIR =" ${PROJECT_SOURCE_DIR})
#message("CMAKE_SOURCE_DIR =" ${CMAKE_SOURCE_DIR})

include_directories(${PROJECT_SOURCE_DIR}
                    ${PROJECT_SOURCE_DIR}/..
                    ${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/dl_inferer/include
                    ${PROJECT_SOURCE_DIR}/post_process/include
                    ${PROJECT_SOURCE_DIR}/pre_process/include
                    ${PROJECT_SOURCE_DIR}/../dl_inferer/include
                    ${PROJECT_SOURCE_DIR}/../post_process/include
                    ${PROJECT_SOURCE_DIR}/../pre_process/include
                    ${PROJECT_SOURCE_DIR}/../../dl_inferer/include
                    ${PROJECT_SOURCE_DIR}/../../post_process/include
                    ${PROJECT_SOURCE_DIR}/../../pre_process/include
                    SYSTEM ${TARGET_FS}/usr/local/include
                    SYSTEM ${TARGET_FS}/usr/include/gstreamer-1.0/
                    SYSTEM ${TARGET_FS}/usr/include/glib-2.0/
                    SYSTEM ${TARGET_FS}/usr/lib/aarch64-linux-gnu/glib-2.0/include
                    SYSTEM ${TARGET_FS}/usr/include/opencv4/
                    SYSTEM ${TARGET_FS}/usr/include/processor_sdk/vision_apps/
                    SYSTEM ${TARGET_FS}/usr/include/edgeai-apps-utils/
                    )

if(USE_DLR_RT)
include_directories(${TARGET_FS}/usr/include/neo-ai-dlr/include)
include_directories(${TARGET_FS}/usr/lib/python3.12/site-packages/dlr/include/)
endif()

if(USE_TENSORFLOW_RT)
include_directories(SYSTEM ${TENSORFLOW_INSTALL_DIR}
                    SYSTEM ${TENSORFLOW_INSTALL_DIR}/lite/tools/make/downloads/flatbuffers/include
                    SYSTEM ${TENSORFLOW_INSTALL_DIR}/lite/tools/pip_package/gen/tflite_pip/python3/cmake_build/flatbuffers/include
                    SYSTEM ${TENSORFLOW_INSTALL_DIR}/tensorflow/lite/tools/make/downloads/flatbuffers/include
                    SYSTEM ${TENSORFLOW_INSTALL_DIR}/tensorflow/lite/tools/pip_package/gen/tflite_pip/python3/cmake_build/flatbuffers/include
                   )
endif()

if(USE_ONNX_RT)
include_directories(SYSTEM ${ONNXRT_INSTALL_DIR}/include/onnxruntime
                    SYSTEM ${ONNXRT_INSTALL_DIR}/include/onnxruntime/core/session
                    )
endif()

set(COMMON_LINK_LIBS
    edgeai_dl_inferer
    edgeai_post_process
    edgeai_pre_process
    edgeai-apps-utils
    )

set(SYSTEM_LINK_LIBS
    yaml-cpp
    pthread
    dl
    )

if ("${TARGET_SOC}" STREQUAL "J721E" OR "${TARGET_SOC}" STREQUAL "J721S2" OR "${TARGET_SOC}" STREQUAL "J722S" OR "${TARGET_SOC}" STREQUAL "J784S4" OR "${TARGET_SOC}" STREQUAL "AM62A")
    list(APPEND
         SYSTEM_LINK_LIBS
         tivision_apps)
endif()


if(USE_DLR_RT)
set(SYSTEM_LINK_LIBS ${SYSTEM_LINK_LIBS} dlr)
endif()

if(USE_TENSORFLOW_RT)
set(SYSTEM_LINK_LIBS ${SYSTEM_LINK_LIBS} tensorflow-lite)
set(SYSTEM_LINK_LIBS ${SYSTEM_LINK_LIBS}
                     flatbuffers
                     fft2d_fftsg2d
                     fft2d_fftsg
                     cpuinfo
                     farmhash
                     ruy_allocator
                     ruy_apply_multiplier
                     ruy_blocking_counter
                     ruy_block_map
                     ruy_context
                     ruy_context_get_ctx
                     ruy_cpuinfo
                     ruy_ctx
                     ruy_denormal
                     ruy_frontend
                     ruy_have_built_path_for_avx2_fma
                     ruy_have_built_path_for_avx512
                     ruy_have_built_path_for_avx
                     ruy_kernel_arm
                     ruy_kernel_avx2_fma
                     ruy_kernel_avx512
                     ruy_kernel_avx
                     ruy_pack_arm
                     ruy_pack_avx2_fma
                     ruy_pack_avx512
                     ruy_pack_avx
                     ruy_prepacked_cache
                     ruy_prepare_packed_matrices
                     ruy_system_aligned_alloc
                     ruy_thread_pool
                     ruy_trmul
                     ruy_tune
                     ruy_wait
                     pthreadpool
                     #xnn lib
                     XNNPACK
)
endif()

if(USE_ONNX_RT)
set(SYSTEM_LINK_LIBS ${SYSTEM_LINK_LIBS} onnxruntime)
endif()

set(SYSTEM_LINK_LIBS ${SYSTEM_LINK_LIBS} pthread dl)

# Function for building a node:
# app_name: app name
# ${ARGN} expands everything after the last named argument to the end
# usage: build_app(app_name a.c b.c....)
function(build_app app_name)
    add_executable(${app_name} ${ARGN})
    target_link_libraries(${app_name}
                          -Wl,--start-group
                          ${COMMON_LINK_LIBS}
                          ${TARGET_LINK_LIBS}
                          ${SYSTEM_LINK_LIBS}
                          -Wl,--end-group
                         )

    set(BIN_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})

    #FILE(GLOB BINS ${CMAKE_CURRENT_SOURCE_DIR}/../bin/${CMAKE_BUILD_TYPE}/*)

    #install(FILES ${BINS}
    #        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    #        DESTINATION ${BIN_INSTALL_DIR})

endfunction()

# Function for building an library
# lib_name: Name of the library
# lib_type: (STATIC, SHARED)
# lib_ver: Version string of the library
# ${ARGN} expands everything after the last named argument to the end
# usage: build_lib(lib_name lib_type lib_ver a.c b.c ....)
function(build_lib lib_name lib_type lib_ver)
    add_library(${lib_name} ${lib_type} ${ARGN})

    SET_TARGET_PROPERTIES(${lib_name}
                          PROPERTIES
                          VERSION ${lib_ver}
                         )

    set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/edgeai_dl_inferer)

    FILE(GLOB HDRS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

    install(TARGETS ${lib_name}
            EXPORT ${lib_name}Targets
            LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}  # Shared Libs
            ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}  # Static Libs
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR}  # Executables, DLLs
           )

    # Specify the header files to install
    install(FILES ${HDRS} DESTINATION ${INCLUDE_INSTALL_DIR})

##    set(CONFIG_PACKAGE_LOCATION ${CMAKE_INSTALL_PREFIX}/cmake/${lib_name})
##
##    # Create config file
##    configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/${lib_name}_config.cmake.in
##        ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}Config.cmake
##        INSTALL_DESTINATION ${CONFIG_PACKAGE_LOCATION}
##        PATH_VARS INCLUDE_INSTALL_DIR
##    )
##
##    install(EXPORT ${lib_name}Targets
##        FILE ${lib_name}Targets.cmake
##        DESTINATION ${CONFIG_PACKAGE_LOCATION}
##    )
##
##    export(EXPORT ${lib_name}Targets
##        FILE ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}Targets.cmake
##    )
##
##    # Generate the version file for the config file
##    write_basic_package_version_file(
##        ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}ConfigVersion.cmake
##        VERSION "${lib_ver}"
##        COMPATIBILITY AnyNewerVersion
##    )
##
##    # install the package configuration files
##    install(FILES
##      ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}Config.cmake
##      ${CMAKE_CURRENT_BINARY_DIR}/${lib_name}ConfigVersion.cmake
##      DESTINATION ${CONFIG_PACKAGE_LOCATION}
##    )

endfunction()

# Function for Installing a Python module
# module_name: Name of the module
function(install_python_module module_name)

    FILE(GLOB MODULE ${CMAKE_CURRENT_SOURCE_DIR}/${module_name}.py)

    set(PYTHON_MAJOR_VERSION "3")
    set(PYTHON_MINOR_VERSION "12")
    set(CMAKE_INSTALL_PYTHON_LIBDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/python${PYTHON_MAJOR_VERSION}.${PYTHON_MINOR_VERSION}/site-packages)

    install(FILES ${MODULE} DESTINATION ${CMAKE_INSTALL_PYTHON_LIBDIR})

endfunction()
