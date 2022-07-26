//
// Created by gaojian on 2022/7/25.
//

#include "Connector.h"
#include <cassert>
#include "Application.h"
#include "support/Util.h"
#include "epoll/handlers/SocketConnectorHandler.h"
#include "eventloop/EventLoopManager.h"
#include "eventloop/EventLoop.h"

namespace netpp {
Connector::Connector()
	: _connectorLoop{nullptr}
{
	APPLICATION_INSTANCE_REQUIRED();
}

void Connector::connect(const Address &address)
{
	_connectorLoop = Application::loopManager()->dispatch();
	auto connector = std::make_shared<SocketConnectorHandler>(_connectorLoop);
	_connector = connector;
	connector->connect(address);
	_connectorLoop->addEventHandlerToLoop(connector);
}

void Connector::setConnectedCallBack(const ConnectedCallBack &cb)
{
	auto connector = _connector.lock();
	if (connector)
		connector->setConnectedCallBack(cb);
}

void Connector::setErrorCallBack(const ErrorCallBack &cb)
{
	auto connector = _connector.lock();
	if (connector)
		connector->setErrorCallBack(cb);
}
}
