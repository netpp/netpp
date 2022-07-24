cmake_minimum_required(VERSION 3.18)
project(example_daytime)

set(CMAKE_CXX_STANDARD 20)

add_executable(example_daytime daytime.cpp)
target_link_libraries(example_daytime netpp pthread)
