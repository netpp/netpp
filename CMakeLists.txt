cmake_minimum_required(VERSION 3.18)
project(netpp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion")

option(NETPP_ENABLE_TEST "build test" ON)
option(NETPP_LOG_SUPPORT "enable log output" ON)
option(NETPP_HTTP_SUPPORT "enable http support" ON)
option(NETPP_EXAMPLE "build examples" ON)

set(netpp_source
include/internal/epoll/Epoll.h include/internal/epoll/EpollEvent.h include/internal/epoll/EventHandler.h
include/internal/handlers/Acceptor.h include/internal/handlers/Connector.h include/internal/handlers/TcpConnection.h include/internal/handlers/TimerHandler.h include/internal/handlers/SignalHandler.h include/internal/handlers/RunInLoopHandler.h
include/support/ThreadSafeQueue.hpp include/internal/support/Log.h
include/internal/socket/Socket.h include/internal/socket/SocketIO.h
include/internal/stub/IO.h include/internal/stub/Socket.h include/internal/stub/Epoll.h
include/internal/time/TimeWheel.h
include/internal/buffer/CowLink.hpp include/internal/buffer/BufferNode.h
include/signal/Signals.h include/signal/SignalWatcher.h
include/error/Exception.h include/error/SocketError.h
include/support/ThreadPool.hpp
include/time/Timer.h
include/Address.h include/ByteArray.h include/Channel.h include/EventLoop.h include/EventLoopDispatcher.h include/Events.h include/TcpClient.h include/TcpServer.h include/Config.h
src/internal/epoll/Epoll.cpp src/internal/epoll/EpollEvent.cpp
src/internal/handlers/Acceptor.cpp src/internal/handlers/Connector.cpp src/internal/handlers/TcpConnection.cpp src/internal/handlers/TimerHandler.cpp src/internal/handlers/SignalHandler.cpp src/internal/handlers/RunInLoopHandler.cpp
src/internal/support/Log.cpp
src/internal/socket/Socket.cpp src/internal/socket/SocketIO.cpp
src/internal/stub/IO.cpp src/internal/stub/Socket.cpp src/internal/stub/Epoll.cpp
src/internal/time/TimeWheel.cpp
src/internal/buffer/BufferNode.cpp
src/signal/Signals.cpp src/signal/SignalWatcher.cpp
src/error/Exception.cpp src/error/SocketError.cpp
src/support/ThreadPool.cpp
src/time/Timer.cpp
src/Address.cpp src/ByteArray.cpp src/Channel.cpp src/EventLoop.cpp src/EventLoopDispatcher.cpp src/TcpClient.cpp src/TcpServer.cpp src/Events.cpp include/support/PooledTask.hpp)

include_directories(${PROJECT_SOURCE_DIR}/include)

if(NETPP_EXAMPLE)
    message(STATUS "netpp: build example")
    add_subdirectory(example)
endif()

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
            thirdParty/llhttp/include/llhttp.h thirdParty/llhttp/src/api.c thirdParty/llhttp/src/http.c thirdParty/llhttp/src/llhttp.c
            include/http/HttpCode.h src/http/HttpRequest.cpp include/http/HttpRequest.h src/http/HttpClient.cpp include/http/HttpClient.h src/http/HttpServer.cpp include/http/HttpServer.h src/http/HttpCode.cpp
            src/internal/http/Route.cpp include/internal/http/Route.h src/internal/http/HttpParser.cpp include/internal/http/HttpParser.h)
    set(netpp_source ${netpp_source} ${netpp_http_source})
    set(netpp_http_test_source http/HttpRequestParseTest.cpp)
    include_directories(${PROJECT_SOURCE_DIR}/thirdParty/llhttp/include)
endif()

if(NETPP_ENABLE_TEST)
    message(STATUS "netpp: build test")
    add_subdirectory(test)
endif()

add_library(netpp SHARED ${netpp_source})
add_library(netpp_static STATIC ${netpp_source})
SET_TARGET_PROPERTIES(netpp_static PROPERTIES OUTPUT_NAME "netpp")
