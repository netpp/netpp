cmake_minimum_required(VERSION 3.16)
project(netpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion")

option(ENABLE_TEST "enable test" ON)
option(USE_LOG "enable log" ON)

add_subdirectory(example)
include_directories(${PROJECT_SOURCE_DIR}/include)

if(ENABLE_TEST STREQUAL ON)
    message(STATUS "Netpp: testing is enabled, add target test")
    add_subdirectory(test)
endif()

if(USE_LOG STREQUAL ON)
    message(STATUS "Netpp: logging is enabled")
    add_definitions()
    add_subdirectory(thirdParty/spdlog)
    include_directories(${PROJECT_SOURCE_DIR}/thirdParty/spdlog/include)
endif()

set(netpp_source
include/internal/epoll/Epoll.h include/internal/epoll/EpollEvent.h include/internal/epoll/EventHandler.h
include/internal/handlers/Acceptor.h include/internal/handlers/Connector.h include/internal/handlers/TcpConnection.h include/internal/handlers/TimerHandler.h include/internal/handlers/SignalHandler.h include/internal/handlers/RunInLoopHandler.h
include/internal/support/ThreadBase.hpp include/internal/support/Log.h
include/internal/socket/Socket.h include/internal/socket/SocketIO.h
include/internal/stub/IO.h include/internal/stub/Socket.h include/internal/stub/Epoll.h
include/internal/time/TimeWheel.h
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
src/signal/Signals.cpp src/signal/SignalWatcher.cpp
src/error/Exception.cpp src/error/SocketError.cpp
src/support/ThreadPool.cpp
src/time/Timer.cpp
src/Address.cpp src/ByteArray.cpp src/Channel.cpp src/EventLoop.cpp src/EventLoopDispatcher.cpp src/TcpClient.cpp src/TcpServer.cpp src/Events.cpp)

add_library(netpp SHARED ${netpp_source})
add_library(netpp_static STATIC ${netpp_source})
SET_TARGET_PROPERTIES(netpp_static PROPERTIES OUTPUT_NAME "netpp")
