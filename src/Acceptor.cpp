//
// Created by gaojian on 22-7-24.
//

#include "Acceptor.h"
#include "support/Util.h"
#include "Application.h"
#include "epoll/handlers/SocketAcceptorHandler.h"
#include "eventloop/EventLoopManager.h"
#include "eventloop/EventLoop.h"
#include <cassert>

namespace netpp {
Acceptor::Acceptor()
	: _acceptorLoop{nullptr}
{
	APPLICATION_INSTANCE_REQUIRED();
}

Acceptor::~Acceptor()
{
	auto acceptor = _acceptor.lock();
	if (acceptor)
		_acceptorLoop->removeEventHandlerFromLoop(acceptor);
}

void Acceptor::listen(const Address &address)
{
	_acceptorLoop = Application::loopManager()->dispatch();
	auto acceptor = std::make_shared<SocketAcceptorHandler>(_acceptorLoop);
	if (m_connectedCb)
		acceptor->setConnectedCallback(m_connectedCb);
	if (m_errorCb)
		acceptor->setErrorCallback(m_errorCb);
	_acceptor = acceptor;
	_acceptorLoop->addEventHandlerToLoop(acceptor);
	acceptor->listen(address);
}

void Acceptor::setConnectedCallback(const ConnectedCallBack &cb)
{
	// acceptor can be nullptr before listen
	auto acceptor = _acceptor.lock();
	if (acceptor)
		acceptor->setConnectedCallback(cb);
	else
		m_connectedCb = cb;
}

void Acceptor::setErrorCallback(const ErrorCallBack &cb)
{
	auto acceptor = _acceptor.lock();
	if (acceptor)
		acceptor->setErrorCallback(cb);
	else
		m_errorCb = cb;
}
}
