cmake_minimum_required(VERSION 3.16)
project(example_timer)

add_executable(example_timer timer.cpp)
target_link_libraries(example_timer netpp pthread)
