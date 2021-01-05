cmake_minimum_required(VERSION 3.16)
project(daytime)

add_executable(daytime daytime.cpp)
target_link_libraries(daytime netpp pthread)
