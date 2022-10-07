cmake_minimum_required(VERSION 3.18)
project(netpp)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Werror")

option(NETPP_LOG_SUPPORT "enable log output" OFF)
option(NETPP_HTTP_SUPPORT "enable http support" OFF)

set(netpp_source
        include/buffer/BufferGather.h include/buffer/Buffer.h
        include/channel/Channel.h include/channel/TcpChannel.h
        include/error/Error.h include/error/Exception.h
        include/eventloop/EventLoop.h include/eventloop/EventLoopData.h include/eventloop/EventLoopFactory.h include/eventloop/EventLoopManager.h
        include/epoll/Epoll.h include/epoll/EpollEventHandler.h
        include/epoll/handlers/SocketAcceptorHandler.h include/epoll/handlers/SocketConnectorHandler.h include/epoll/handlers/SocketConnectionHandler.h include/epoll/handlers/TimerHandler.h include/epoll/handlers/SignalHandler.h include/epoll/handlers/RunInLoopHandler.h
        include/iodevice/IODevice.h include/iodevice/SocketDevice.h include/iodevice/TcpSocket.h include/iodevice/UdpSocket.h
        include/location/uri/Uri.h include/location/uri/UriCodec.h
        include/location/Address.h
        include/signal/Signals.h
        include/support/ThreadSafeQueue.hpp include/support/Log.h include/support/ThreadPool.hpp include/support/PooledTask.hpp include/support/Util.h include/support/Types.h
        include/time/TimerData.h include/time/Timer.h include/time/TimeWheel.h include/time/TickTimer.h
        include/Application.h include/Acceptor.h include/Connector.h

        src/buffer/BufferGather.cpp src/buffer/Buffer.cpp
        src/channel/Channel.cpp src/channel/TcpChannel.cpp
        src/error/Error.cpp src/error/Exception.cpp
        src/eventloop/EventLoop.cpp src/eventloop/EventLoopData.cpp src/eventloop/EventLoopFactory.cpp src/eventloop/EventLoopManager.cpp
        src/epoll/Epoll.cpp src/epoll/EpollEventHandler.cpp
        src/epoll/handlers/SocketAcceptorHandler.cpp src/epoll/handlers/SocketConnectorHandler.cpp src/epoll/handlers/SocketConnectionHandler.cpp src/epoll/handlers/TimerHandler.cpp src/epoll/handlers/SignalHandler.cpp src/epoll/handlers/RunInLoopHandler.cpp
        src/iodevice/SocketDevice.cpp src/iodevice/TcpSocket.cpp src/iodevice/UdpSocket.cpp
        src/location/uri/Uri.cpp
        src/location/Address.cpp
        src/signal/Signals.cpp
        src/support/Log.cpp src/support/ThreadPool.cpp src/support/Util.cpp
        src/time/Timer.cpp src/time/TimeWheel.cpp src/time/TickTimer.cpp
        src/Application.cpp src/Acceptor.cpp src/Connector.cpp
        src/buffer/ByteArray.cpp include/buffer/ByteArray.h)

include_directories(${PROJECT_SOURCE_DIR}/include)

if(NETPP_LOG_SUPPORT)
    message(STATUS "netpp: enable log output")
    add_definitions()
    add_subdirectory(thirdParty/spdlog)
    include_directories(${PROJECT_SOURCE_DIR}/thirdParty/spdlog/include)
endif()

if(NETPP_HTTP_SUPPORT)
    message(STATUS "netpp: enable http support")
    # not use llhttp's cmake, avoid link library
    set(netpp_http_source
            include/http/HttpCode.h include/http/HttpRequest.h include/http/HttpClient.h include/http/HttpServer.h
            src/http/HttpRequest.cpp src/http/HttpClient.cpp src/http/HttpServer.cpp src/http/HttpCode.cpp
            include/internal/http/Route.h include/internal/http/HttpPackage.h
            src/internal/http/Route.cpp src/internal/http/HttpPackage.cpp src/internal/http/channel/HttpChannelConversion.cpp
            include/buffer/HttpChannel.h src/buffer/HttpChannel.cpp src/internal/http/handler/ServerEvents.cpp include/internal/http/handler/ServerEvents.h src/internal/http/handler/ClientEvents.cpp include/internal/http/handler/ClientEvents.h)
    set(netpp_source ${netpp_source} ${netpp_http_source})
    set(netpp_http_test_source http/HttpRequestParseTest.cpp http/HttpResponseParseTest.cpp http/HttpBaseRequestTest.cpp)
    include_directories(${PROJECT_SOURCE_DIR}/thirdParty/llhttp/include)
    add_subdirectory(thirdParty/llhttp)
    target_compile_options(llhttp PRIVATE -fPIC)
endif()

add_subdirectory(example)
add_subdirectory(test)
add_subdirectory(doc)

add_library(netpp SHARED ${netpp_source})

add_library(netpp_static STATIC ${netpp_source})
target_link_libraries(netpp_static)
SET_TARGET_PROPERTIES(netpp_static PROPERTIES OUTPUT_NAME "netpp")
