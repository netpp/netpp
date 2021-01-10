//
// Created by gaojian on 2020/7/6.
//

#include "TcpServer.h"
#include "EventLoop.h"
#include "handlers/Acceptor.h"

namespace netpp {
TcpServer::TcpServer(EventLoopDispatcher *dispatcher, std::unique_ptr<Events> &&eventsPrototype)
	: _acceptorChannel{nullptr}, _dispatcher(dispatcher),
	  _eventPrototype{std::move(eventsPrototype)}
{}

void TcpServer::listen(Address listenAddr)
{
	handlers::Acceptor::makeAcceptor(_dispatcher, listenAddr, std::move(_eventPrototype));
	_eventPrototype = nullptr;
}
}
