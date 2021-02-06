//
// Created by gaojian on 2020/7/20.
//

#ifndef NETPP_TCPCLIENT_H
#define NETPP_TCPCLIENT_H

#include "Address.h"
#include "Events.h"

namespace netpp {
namespace handlers {
class Connector;
}
class EventLoopDispatcher;
class TcpClient {
public:
	TcpClient(EventLoopDispatcher *dispatcher, Address addr, std::unique_ptr<support::EventInterface> &&eventsPrototype);
	~TcpClient();

	void connect();
	void disconnect();

private:
	Address m_addr;
	std::shared_ptr<handlers::Connector> m_connector;

	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<support::EventInterface> m_eventsPrototype;
};
}

#endif //NETPP_TCPCLIENT_H
