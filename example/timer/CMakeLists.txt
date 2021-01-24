cmake_minimum_required(VERSION 3.16)
project(timer)

add_executable(timer timer.cpp)
target_link_libraries(timer netpp pthread)
