#ifndef NETPP_CONNECTOR_H
#define NETPP_CONNECTOR_H

#include <memory>
#include "epoll/EventHandler.h"
#include "socket/Socket.h"
#include "socket/SocketEnums.h"
#include "time/Timer.h"
#include "Events.h"
#include "Address.h"

namespace netpp {
class EventLoopDispatcher;
}

namespace netpp::handlers {
class TcpConnection;

/**
 * @brief 
 */
class Connector : public epoll::EventHandler, public std::enable_shared_from_this<Connector> {
public:
	Connector(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket);
	~Connector() override = default;

	/**
	 * @brief try to connect to peer, Connector will be removed from EventLoop after connect success
	 */
	void connect();

	/**
	 * @brief if connecting, stop retry to connect, if already connected, do nothing
	 */
	void stopConnect();

	/**
	 * @brief get the TcpConnection connected by Connector
	 */
	std::weak_ptr<TcpConnection> getConnection() const { return _connection; }
	bool connected() const;

	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleClose() override;

	static std::weak_ptr<Connector> makeConnector(EventLoopDispatcher *dispatcher,
								  Address serverAddr,
								  std::unique_ptr<support::EventInterface> &&eventsPrototype);

private:
	void setupTimer();
	void reconnect();

	bool m_isConnected;
	std::weak_ptr<TcpConnection> _connection;

	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<socket::Socket> m_socket;
	std::unique_ptr<time::Timer> m_retryTimer;
	std::unique_ptr<support::EventInterface> m_events;
};
}

#endif
