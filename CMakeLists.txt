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
include/handlers/Acceptor.h include/handlers/Connector.h include/handlers/TcpConnection.h include/handlers/TimerHandler.h include/handlers/SignalHandler.h
include/support/ThreadBase.hpp include/support/ThreadPool.hpp include/support/EventsInterface.h
include/signal/Signals.h include/signal/SignalWatcher.h include/signal/SignalFd.h
include/socket/Socket.h include/socket/SocketIO.h
include/stub/IO.h include/stub/Socket.h
include/error/Exception.h include/error/SocketError.h
include/time/Timer.h include/time/TimeWheel.h
include/Address.h include/ByteArray.h include/Channel.h include/EventLoop.h include/EventLoopDispatcher.h include/Events.h include/Log.h include/TcpClient.h include/TcpServer.h
src/epoll/Epoll.cpp src/epoll/EpollEvent.cpp
src/handlers/Acceptor.cpp src/handlers/Connector.cpp src/handlers/TcpConnection.cpp src/handlers/TimerHandler.cpp src/handlers/SignalHandler.cpp
src/support/ThreadPool.cpp
src/signal/Signals.cpp src/signal/SignalWatcher.cpp src/signal/SignalFd.cpp
src/socket/Socket.cpp src/socket/SocketIO.cpp
src/stub/IO.cpp src/stub/Socket.cpp
src/error/Exception.cpp src/error/SocketError.cpp
src/time/Timer.cpp src/time/TimeWheel.cpp
src/Address.cpp src/ByteArray.cpp src/Channel.cpp src/EventLoop.cpp src/EventLoopDispatcher.cpp src/Log.cpp src/TcpClient.cpp src/TcpServer.cpp)
