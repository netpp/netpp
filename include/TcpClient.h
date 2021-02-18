//
// Created by gaojian on 2020/7/20.
//

#ifndef NETPP_TCPCLIENT_H
#define NETPP_TCPCLIENT_H

#include "Address.h"
#include "Events.h"

namespace netpp {
namespace internal::handlers {
class Connector;
}
class EventLoopDispatcher;
class TcpClient {
public:
	TcpClient(EventLoopDispatcher *dispatcher, Address addr, Events eventsPrototype);
	~TcpClient();

	void connect();
	void disconnect();

private:
	Address m_addr;
	std::shared_ptr<internal::handlers::Connector> m_connector;

	EventLoopDispatcher *_dispatcher;
	Events m_eventsPrototype;
};
}

#endif //NETPP_TCPCLIENT_H
