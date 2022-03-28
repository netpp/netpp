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
/**
 * @brief The top level tcp client, a client can connection to only one server
 */
class TcpClient {
public:
	/**
	 * @brief Create a tcp client
	 * @param dispatcher		event loop dispatcher
	 * @param addr				address of server
	 * @param eventsPrototype	user-defined events handler
	 */
	TcpClient(EventLoopDispatcher *dispatcher, Address addr, Events eventsPrototype);
	~TcpClient();

	/**
	 * @brief Connect to server async
	 */
	void connect();
	/**
	 * @brief Disconnect from server async
	 */
	void disconnect();

private:
	Address m_addr;
	std::shared_ptr<internal::handlers::Connector> m_connector;

	EventLoopDispatcher *_dispatcher;
	Events m_eventsPrototype;
};
}

#endif //NETPP_TCPCLIENT_H
