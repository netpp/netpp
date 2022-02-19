cmake_minimum_required(VERSION 3.18)
project(example_timer)

add_executable(example_timer timer.cpp)
target_link_libraries(example_timer netpp pthread)
