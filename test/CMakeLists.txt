cmake_minimum_required(VERSION 3.18)
project(test)

add_subdirectory(${PROJECT_SOURCE_DIR}/../thirdParty/googletest thirdParty/googletest)

include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googlemock/include)

set(request_test
        http_request_v1_0
        http_request_v1_1
        http_request_v1_4
        http_request_v2_0
        http_request_without_header
        http_request_header
        http_request_header_without_body
        )
foreach(rq ${request_test})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/http/requests/${rq} ${CMAKE_CURRENT_BINARY_DIR}/requests/${rq} COPYONLY)
endforeach()

link_libraries(gtest gmock pthread dl netpp)
add_executable(test ${netpp_http_test_source}
main.cpp
MockSysCallEnvironment.h MockSysCallEnvironment.cpp
ByteArray/ByteArrayReaderTest.cpp ByteArray/ByteArraySequentialReaderTest.cpp ByteArray/ByteArrayTest.cpp ByteArray/ByteArrayWriterTest.cpp 
SocketAndAddressTest.cpp ThreadPoolTest.cpp EpollEventTest.cpp SignalTest.cpp TimerTest.cpp StubTest.cpp AcceptorTest.cpp RunInLoopTest.cpp ConnectorTest.cpp ConnectionTest.cpp ByteArray/CowLinkTest.cpp)
