//
// Created by gaojian on 2020/7/6.
//

#include "TcpServer.h"
#include "EventLoop.h"
#include "handlers/Acceptor.h"
#include "EventLoopDispatcher.h"

namespace netpp {
TcpServer::TcpServer(EventLoopDispatcher *dispatcher, Address addr, Events eventsPrototype)
	: m_addr{addr}, _dispatcher(dispatcher), m_eventPrototype{eventsPrototype}
{}

TcpServer::~TcpServer()
{
	// TODO: disconnect all clients
	stopListen();
}

void TcpServer::listen()
{
	auto acceptor = internal::handlers::Acceptor::makeAcceptor(_dispatcher, m_addr, m_eventPrototype);
	_acceptor = acceptor;
	if (acceptor)
		acceptor->listen();
}

void TcpServer::stopListen()
{
	auto acceptor = _acceptor.lock();
	if (acceptor)
		acceptor->stop();
}
}
