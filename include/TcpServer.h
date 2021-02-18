//
// Created by gaojian on 2020/7/6.
//

#ifndef NETPP_TCPSERVER_H
#define NETPP_TCPSERVER_H

#include "Events.h"
#include "Address.h"

namespace netpp {
namespace internal::handlers {
class Acceptor;
}
class EventLoopDispatcher;

class TcpServer {
public:
	TcpServer(EventLoopDispatcher *dispatcher, Address addr, 
		Events eventsPrototype);
	~TcpServer();

	void listen();
	void stopListen();

private:
	Address m_addr;
	std::weak_ptr<internal::handlers::Acceptor> _acceptor;
	
	EventLoopDispatcher *_dispatcher;
	Events m_eventPrototype;
};
}

#endif //NETPP_TCPSERVER_H
