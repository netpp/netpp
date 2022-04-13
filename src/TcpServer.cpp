//
// Created by gaojian on 2020/7/6.
//

#include "TcpServer.h"
#include "eventloop/EventLoop.h"
#include "internal/handlers/Acceptor.h"
#include "Application.h"

namespace netpp {
TcpServer::TcpServer(Address address)
	: m_address{std::move(address)}
{
	Config config = Application::appConfig();
	m_config = config.connection;
	m_eventPrototype = config.eventHandler;
}

TcpServer::~TcpServer()
{
	// TODO: disconnect all clients
	stopListen();
}

void TcpServer::listen()
{
	auto acceptor = internal::handlers::Acceptor::makeAcceptor(Application::loopManager()->dispatch(), m_address, m_eventPrototype, m_config);
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
