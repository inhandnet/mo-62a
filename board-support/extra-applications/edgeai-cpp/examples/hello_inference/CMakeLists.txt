# Minimal Edge AI inference example for MO-62A.
# Build on the board:  mkdir build && cd build && cmake .. && make
cmake_minimum_required(VERSION 3.16)
project(hello_inference CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# The MO-62A SDK installs EdgeAIConfig.cmake to /usr/lib/cmake/EdgeAI.
find_package(EdgeAI REQUIRED)

add_executable(hello_inference src/hello_inference.cpp)
target_link_libraries(hello_inference PRIVATE EdgeAI::edgeai)
