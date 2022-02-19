cmake_minimum_required(VERSION 3.18)
project(example_signal_handle)

add_executable(example_signal_handle signal_handle.cpp)
target_link_libraries(example_signal_handle netpp pthread)
