cmake_minimum_required(VERSION 3.18)
project(test)

add_subdirectory(${PROJECT_SOURCE_DIR}/../thirdParty/googletest thirdParty/googletest)

include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googlemock/include)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/http/requests/http_request_v1_0 ${CMAKE_CURRENT_BINARY_DIR}/requests COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/http/requests/http_request_v1_1 ${CMAKE_CURRENT_BINARY_DIR}/requests COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/http/requests/http_request_v1_4 ${CMAKE_CURRENT_BINARY_DIR}/requests COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/http/requests/http_request_v2_0 ${CMAKE_CURRENT_BINARY_DIR}/requests COPYONLY)

link_libraries(gtest gmock pthread dl netpp)
add_executable(test ${netpp_http_test_source}
main.cpp
MockSysCallEnvironment.h MockSysCallEnvironment.cpp
ByteArray/ByteArrayReaderTest.cpp ByteArray/ByteArraySequentialReaderTest.cpp ByteArray/ByteArrayTest.cpp ByteArray/ByteArrayWriterTest.cpp 
SocketAndAddressTest.cpp ThreadPoolTest.cpp EpollEventTest.cpp SignalTest.cpp TimerTest.cpp StubTest.cpp AcceptorTest.cpp RunInLoopTest.cpp ConnectorTest.cpp ConnectionTest.cpp)

#add_custom_command(TARGET ${PROJECT_NAME}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E
#        copy_if_different
#        "${PROJECT_SOURCE_DIR}/requests"
#        "${CMAKE_BINARY_DIR}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/Plugins/org_test_plugins/"
#        )
