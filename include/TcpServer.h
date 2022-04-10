//
// Created by gaojian on 2020/7/6.
//

#ifndef NETPP_TCPSERVER_H
#define NETPP_TCPSERVER_H

#include "Events.h"
#include "Address.h"
#include "Config.h"

namespace netpp {
namespace internal::handlers {
class Acceptor;
}
/**
 * @brief The top level tcp server
 */
class TcpServer {
public:
	/**
	 * @brief Create a tcp server
	 * @param address			address of server
	 * @param eventsPrototype	user-defined events handler
	 */
	explicit TcpServer(Address address);
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
	Address m_address;
	std::weak_ptr<internal::handlers::Acceptor> _acceptor;
	ConnectionConfig m_config;

	Events m_eventPrototype;
};
}

#endif //NETPP_TCPSERVER_H
