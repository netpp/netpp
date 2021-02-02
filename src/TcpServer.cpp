//
// Created by gaojian on 2020/7/6.
//

#include "TcpServer.h"
#include "EventLoop.h"
#include "handlers/Acceptor.h"
#include "EventLoopDispatcher.h"

namespace netpp {
TcpServer::TcpServer(EventLoopDispatcher *dispatcher, Address addr, std::unique_ptr<support::EventInterface> &&eventsPrototype)
	: _dispatcher(dispatcher), m_addr{addr}, _eventPrototype{std::move(eventsPrototype)}
{}

void TcpServer::listen()
{
	handlers::Acceptor::makeAcceptor(_dispatcher, m_addr, _eventPrototype->clone());
	_eventPrototype = nullptr;
}
}
