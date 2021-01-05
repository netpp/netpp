cmake_minimum_required(VERSION 3.16)
project(example)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/thirdParty/spdlog/include)

add_subdirectory(echo)
add_subdirectory(daytime)
