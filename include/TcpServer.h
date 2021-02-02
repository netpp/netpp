//
// Created by gaojian on 2020/7/6.
//

#ifndef NETPP_TCPSERVER_H
#define NETPP_TCPSERVER_H

#include "Events.h"
#include "Address.h"

namespace netpp {
class EventLoopDispatcher;

class TcpServer {
public:
	explicit TcpServer(EventLoopDispatcher *dispatcher, Address addr, 
		std::unique_ptr<support::EventInterface> &&eventsPrototype);

	void listen();

private:
	Address m_addr;
	
	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<support::EventInterface> _eventPrototype;
};
}

#endif //NETPP_TCPSERVER_H
