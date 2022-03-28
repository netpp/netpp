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
/**
 * @brief The top level tcp server
 */
class TcpServer {
public:
	/**
	 * @brief Create a tcp server
	 * @param dispatcher		event loop dispatcher
	 * @param addr				address of server
	 * @param eventsPrototype	user-defined events handler
	 */
	TcpServer(EventLoopDispatcher *dispatcher, Address addr, 
		Events eventsPrototype);
	~TcpServer();

	/**
	 * @brief Start listen connection request, and auto manage clients life cycle
	 */
	void listen();
	/**
	 * @brief Stop accept incoming connections
	 */
	void stopListen();

private:
	Address m_addr;
	std::weak_ptr<internal::handlers::Acceptor> _acceptor;
	
	EventLoopDispatcher *_dispatcher;
	Events m_eventPrototype;
};
}

#endif //NETPP_TCPSERVER_H
