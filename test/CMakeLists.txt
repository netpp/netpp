cmake_minimum_required(VERSION 3.16)
project(test)

include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/spdlog/include)

link_libraries(gtest pthread netpp dl)
add_executable(test main.cpp ByteArrayTest.cpp ThreadPoolTest.cpp HandlerEpollEventTest.cpp)
