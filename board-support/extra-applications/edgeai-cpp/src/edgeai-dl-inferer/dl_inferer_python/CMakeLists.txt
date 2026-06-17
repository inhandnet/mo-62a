cmake_minimum_required(VERSION 3.10.0)

project(edgeai_dl_inferer_python)

include(${CMAKE_SOURCE_DIR}/cmake/common.cmake)

install_python_module(edgeai_dl_inferer) # Named argument: module name
