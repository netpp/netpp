cmake_minimum_required(VERSION 3.16)
project(netpp)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(example)
add_subdirectory(test)
add_subdirectory(thirdParty/spdlog)
add_subdirectory(thirdParty/googletest)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdParty/spdlog/include)

add_library(netpp SHARED
include/epoll/Epoll.h include/epoll/EpollEvent.h include/epoll/EventHandler.h
include/handlers/SocketEventHandler.h include/handlers/TimerHandler.h
include/support/ThreadBase.hpp include/support/ThreadPool.hpp
include/Address.h include/ByteArray.h include/Channel.h include/EventLoop.h include/EventLoopDispatcher.h include/Events.h include/Log.h include/Socket.h include/TcpClient.h include/TcpServer.h include/Timer.h
src/epoll/Epoll.cpp src/epoll/EpollEvent.cpp
src/handlers/SocketEventHandler.cpp src/handlers/TimerHandler.cpp
src/support/ThreadPool.cpp
src/Address.cpp src/ByteArray.cpp src/Channel.cpp src/EventLoop.cpp src/EventLoopDispatcher.cpp src/Log.cpp src/Socket.cpp src/TcpClient.cpp src/TcpServer.cpp src/Timer.cpp)
