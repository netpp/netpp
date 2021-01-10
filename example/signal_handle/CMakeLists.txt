cmake_minimum_required(VERSION 3.16)
project(signal_handle)

add_executable(signal_handle signal_handle.cpp)
target_link_libraries(signal_handle netpp pthread)
