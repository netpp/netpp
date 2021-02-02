//
// Created by gaojian on 2020/7/20.
//

#ifndef NETPP_TCPCLIENT_H
#define NETPP_TCPCLIENT_H

#include "Address.h"
#include "Events.h"

namespace netpp {
class EventLoopDispatcher;
class TcpClient {
public:
	explicit TcpClient(EventLoopDispatcher *dispatcher, Address addr, std::unique_ptr<support::EventInterface> &&eventsPrototype);

	void connect();
	void disconnect();

private:
	Address m_addr;

	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<support::EventInterface> m_eventsPrototype;
};
}

#endif //NETPP_TCPCLIENT_H
