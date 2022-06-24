//
// Created by gaojian on 2020/7/20.
//

#ifndef NETPP_TCPCLIENT_H
#define NETPP_TCPCLIENT_H

#include "Address.h"
#include "Events.h"
#include "Config.h"

namespace netpp {
namespace internal::handlers {
class SocketConnectorHandler;
}
/**
 * @brief The top level tcp client, a client can connection to only one server
 */
class TcpClient {
public:
	/**
	 * @brief Create a tcp client
	 * @param address			address of server
	 * @param eventsPrototype	user-defined events handler
	 */
	explicit TcpClient(Address address);
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
	Address m_address;
	std::shared_ptr<internal::handlers::SocketConnectorHandler> m_connector;
	ConnectionConfig m_config;
	Events m_eventsPrototype;
};
}

#endif //NETPP_TCPCLIENT_H
