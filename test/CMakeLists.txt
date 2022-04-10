cmake_minimum_required(VERSION 3.18)
project(test)

add_subdirectory(${PROJECT_SOURCE_DIR}/../thirdParty/googletest thirdParty/googletest)

include_directories(${PROJECT_SOURCE_DIR}/../include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googletest/include)
include_directories(${PROJECT_SOURCE_DIR}/../thirdParty/googletest/googlemock/include)

set(request_test
        v1_0.http_request
        v1_1.http_request
        v1_4.http_request
        v2_0.http_request
        without_header.http_request
        header.http_request
        header_without_body.http_request
        )
foreach(rq ${request_test})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/http/requests/${rq} ${CMAKE_CURRENT_BINARY_DIR}/requests/${rq} COPYONLY)
endforeach()

set(response_test
        v1_0.http_response
        v1_1.http_response
        v1_4.http_response
        v2_0.http_response
        without_header.http_response
        header.http_response
        header_without_body.http_response
        )
foreach(rq ${response_test})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/http/response/${rq} ${CMAKE_CURRENT_BINARY_DIR}/response/${rq} COPYONLY)
endforeach()

link_libraries(gtest gmock pthread dl netpp)
add_executable(test ${netpp_http_test_source}
        main.cpp
        MockSysCallEnvironment.h MockSysCallEnvironment.cpp
        ByteArray/ByteArrayReaderTest.cpp ByteArray/ByteArraySequentialReaderTest.cpp ByteArray/ByteArrayTest.cpp ByteArray/ByteArrayWriterTest.cpp
        uri/UriAccessor.cpp uri/UriCombining.cpp uri/UriConstructor.cpp uri/UriConversions.cpp uri/UriEncoding.cpp uri/UriOperator.cpp uri/UriResolve.cpp uri/UriSplitting.cpp uri/UriBuilder.cpp
        SocketAndAddressTest.cpp ThreadPoolTest.cpp EpollEventTest.cpp SignalTest.cpp TimerTest.cpp StubTest.cpp handler/AcceptorTest.cpp handler/RunInLoopTest.cpp handler/ConnectorTest.cpp handler/ConnectionTest.cpp ByteArray/CowLinkTest.cpp)
