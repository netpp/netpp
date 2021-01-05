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
	explicit TcpClient(EventLoopDispatcher *dispatcher, std::unique_ptr<Events> &&eventsPrototype);

	void connect(Address serverAddr);
	void disconnect(Address serverAddr);

private:
	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<Events> m_eventsPrototype;
};
}

#endif //NETPP_TCPCLIENT_H
