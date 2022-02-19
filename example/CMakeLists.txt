cmake_minimum_required(VERSION 3.18)
project(example)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(echo)
add_subdirectory(daytime)
add_subdirectory(signal_handle)
add_subdirectory(timer)
add_subdirectory(chat)
