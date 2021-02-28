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

namespace netpp::internal::handlers {
class TcpConnection;

/**
 * @brief The Connector try to connect to a given address
 * connect failed should retry several times, retry interval
 * will double than last one utill a certain max interval
 */
class Connector : public epoll::EventHandler, public std::enable_shared_from_this<Connector> {
public:
	/// @brief Use makeConnector to create a Connector
	Connector(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket);
	~Connector() override = default;

	/**
	 * @brief Try to connect to peer, Connector will be removed from EventLoop after connect success
	 * can be used out side EventLoop, thread safe
	 */
	void connect();

	/**
	 * @brief If connecting, stop retry to connect, if already connected, do nothing
	 * can be used out side EventLoop, thread safe
	 */
	void stopConnect();

	/**
	 * @brief Get the TcpConnection connected by Connector
	 */
	std::weak_ptr<TcpConnection> getConnection() const { return _connection; }

	/**
	 * @brief Is already connected
	 * Can be used out side EventLoop, thread safe
	 */
	bool connected() const { return m_connectionEstablished.load(std::memory_order_relaxed); }
	
	/**
	 * @brief Handle EPOLLOUT triggered when connect state changed,
	 * if connect failed in some situations, should start retry connect.
	 * @note Handlers will run only in EventLoop, NOT thread safe.
	 * @note EPOLLIN should be triggered together with EPOLLOUT in connector
	 * when connect state changed, but we there's no need to handle it again
	 * 
	 */
	void handleOut() override;

	/**
	 * @brief create a new connector, thread safe
	 * 
	 * @param dispatcher				event loop dispatcher, assign connector to an EventLoop
	 * @param serverAddr				address connect to
	 * @param eventsPrototype			user-define event handler
	 * @return std::weak_ptr<Connector>	the Connector created
	 */
	static std::shared_ptr<Connector> makeConnector(EventLoopDispatcher *dispatcher,
								  Address serverAddr,
								  Events eventsPrototype);

private:
	/**
	 * @brief Init retry timer
	 * 
	 */
	void setupTimer();

	/**
	 * @brief Try to reconnect later
	 * 
	 */
	void reconnect();

	std::weak_ptr<TcpConnection> _connection;

	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<socket::Socket> m_socket;
	std::unique_ptr<netpp::time::Timer> m_retryTimer;
	Events m_events;

	/**
	 * @brief the state of connector
	 *           stop()   retry()
	 *           +--<-+   +-<-+
	 *           |    |   |   |
	 * * -> Closed -> Connecting -> Established
	 *  make     connect      connected
	 */
	socket::TcpState m_state;
	std::atomic_bool m_connectionEstablished;
};
}

#endif
