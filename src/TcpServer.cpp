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

TcpServer::~TcpServer()
{
	// TODO: disconnect all clients
	stopListen();
}

void TcpServer::listen()
{
	auto acceptor = handlers::Acceptor::makeAcceptor(_dispatcher, m_addr, _eventPrototype->clone()).lock();
	_acceptor = acceptor;
	acceptor->listen();
	_eventPrototype = nullptr;
}

void TcpServer::stopListen()
{
	auto acceptor = _acceptor.lock();
	if (acceptor)
		acceptor->stop();
}
}
