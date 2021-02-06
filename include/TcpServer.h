//
// Created by gaojian on 2020/7/6.
//

#ifndef NETPP_TCPSERVER_H
#define NETPP_TCPSERVER_H

#include "Events.h"
#include "Address.h"

namespace netpp {
namespace handlers {
class Acceptor;
}
class EventLoopDispatcher;

class TcpServer {
public:
	TcpServer(EventLoopDispatcher *dispatcher, Address addr, 
		std::unique_ptr<support::EventInterface> &&eventsPrototype);
	~TcpServer();

	void listen();
	void stopListen();

private:
	Address m_addr;
	std::weak_ptr<handlers::Acceptor> _acceptor;
	
	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<support::EventInterface> _eventPrototype;
};
}

#endif //NETPP_TCPSERVER_H
