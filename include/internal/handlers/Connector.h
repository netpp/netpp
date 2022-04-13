#ifndef NETPP_CONNECTOR_H
#define NETPP_CONNECTOR_H

#include <memory>
#include <atomic>
#include "internal/epoll/EventHandler.h"
#include "Events.h"
#include "Config.h"

namespace netpp {
class Address;
namespace internal::socket {
class Socket;
enum class TcpState;
}
namespace time {
class Timer;
}
}

namespace netpp::internal::handlers {
class TcpConnection;

/**
 * @brief The Connector try to connect to a given address,
 * connect failed should retry several times, retry interval
 * will double than last one util a certain max interval
 */
class Connector : public epoll::EventHandler, public std::enable_shared_from_this<Connector> {
public:
	/// @brief Use makeConnector to create a Connector
	Connector(eventloop::EventLoop *loop, std::unique_ptr<socket::Socket> &&socket);
	~Connector() override;

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
	 * @brief Create a new connector, thread safe
	 * 
	 * @param dispatcher				Event loop dispatcher, assign connector to an EventLoop
	 * @param serverAddr				Address connect to
	 * @param eventsPrototype			User-define event handler
	 * @return std::weak_ptr<Connector>	The Connector created
	 */
	static std::shared_ptr<Connector> makeConnector(eventloop::EventLoop *loop, const Address &serverAddr,
												 Events eventsPrototype, ConnectionConfig config);

protected:
	/**
	 * @brief Handle EPOLLOUT triggered when connect state changed,
	 * if connect failed in some situations, should start retry connect.
	 * @note Handlers will run only in EventLoop, NOT thread safe.
	 *
	 */
	void handleOut() override;

private:
	/**
	 * @brief Try to reconnect later
	 * 
	 */
	void reconnect();

	std::weak_ptr<TcpConnection> _connection;

	std::unique_ptr<socket::Socket> m_socket;
	std::unique_ptr<netpp::time::Timer> m_retryTimer;
	Events m_events;

	ConnectionConfig m_config;
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
