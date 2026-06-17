cmake_minimum_required(VERSION 3.10.0)

project(edgeai_dl_inferer_lib)

add_subdirectory(dl_inferer)
add_subdirectory(dl_inferer_python)
add_subdirectory(post_process)
add_subdirectory(pre_process)
add_subdirectory(examples)
add_subdirectory(tests/test_cpp)
