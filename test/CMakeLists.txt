cmake_minimum_required(VERSION 3.16)
project(test)

add_subdirectory(${PROJECT_SOURCE_DIR}/../thirdParty/googletest thirdParty/googletest)

include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googlemock/include)

set(test_netpp_source)
foreach (s ${netpp_source})
    set(test_netpp_source ${test_netpp_source} ../${s})
endforeach(s)

link_libraries(gtest gmock pthread dl)
add_executable(test ${test_netpp_source}
main.cpp
MockSysCallEnvironment.h MockSysCallEnvironment.cpp
SocketAndAddressTest.cpp ByteArrayTest.cpp ThreadPoolTest.cpp EpollEventTest.cpp SignalTest.cpp TimerTest.cpp StubTest.cpp AcceptorTest.cpp RunInLoopTest.cpp ConnectorTest.cpp ConnectionTest.cpp)
