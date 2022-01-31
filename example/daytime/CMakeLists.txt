cmake_minimum_required(VERSION 3.16)
project(example_daytime)

add_executable(example_daytime daytime.cpp)
target_link_libraries(example_daytime netpp pthread)
