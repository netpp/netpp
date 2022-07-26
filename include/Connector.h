//
// Created by gaojian on 2022/7/25.
//

#ifndef NETPP_CONNECTOR_H
#define NETPP_CONNECTOR_H

#include <memory>
#include "support/Types.h"

namespace netpp {
class EventLoop;
class Address;
class SocketConnectorHandler;
class Connector {
public:
	Connector();

	void connect(const Address &address);

	void setConnectedCallBack(const ConnectedCallBack &cb);
	void setErrorCallBack(const ErrorCallBack &cb);

private:
	EventLoop *_connectorLoop;
	std::weak_ptr<SocketConnectorHandler> _connector;
};
}

#endif //NETPP_CONNECTOR_H
