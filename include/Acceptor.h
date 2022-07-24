//
// Created by gaojian on 22-7-24.
//

#ifndef NETPP_ACCEPTOR_H
#define NETPP_ACCEPTOR_H

#include <memory>
#include "location/Address.h"
#include "support/Types.h"

namespace netpp {
class SocketAcceptorHandler;
class EventLoop;
class Acceptor {
public:
	Acceptor();
	~Acceptor();

	void listen(const Address &address);

	void setConnectedCallback(const ConnectedCallBack &cb);
	void setErrorCallback(const ErrorCallBack &cb);

private:
	EventLoop *_acceptorLoop;
	std::weak_ptr<SocketAcceptorHandler> _acceptor;

	ConnectedCallBack m_connectedCb;
	ErrorCallBack m_errorCb;
};
}

#endif //NETPP_ACCEPTOR_H
