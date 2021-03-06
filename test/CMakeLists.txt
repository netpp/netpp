cmake_minimum_required(VERSION 3.16)
project(test)

add_subdirectory(${PROJECT_SOURCE_DIR}/../thirdParty/googletest thirdParty/googletest)

include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googlemock/include)

link_libraries(gtest gmock pthread netpp dl)
add_executable(test
main.cpp
MockSysCallEnvironment.h MockSysCallEnvironment.cpp
ByteArrayTest.cpp ThreadPoolTest.cpp EpollEventTest.cpp SignalTest.cpp TimerTest.cpp)
