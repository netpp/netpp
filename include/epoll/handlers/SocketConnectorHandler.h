#ifndef NETPP_SOCKETCONNECTORHANDLER_H
#define NETPP_SOCKETCONNECTORHANDLER_H

#include <memory>
#include <atomic>
#include "epoll/EpollEventHandler.h"
#include "support/Types.h"
#include "location/Address.h"

namespace netpp {
class SocketConnectionHandler;
class SocketDevice;
enum class TcpState;
class Timer;

/**
 * @brief The SocketConnectorHandler try to connect to a given address,
 * connect failed should retry several times, retry interval
 * will double than last one util a certain max interval
 */
class SocketConnectorHandler : public EpollEventHandler, public std::enable_shared_from_this<SocketConnectorHandler> {
public:
	/// @brief Use makeConnector to create a SocketConnectorHandler
	explicit SocketConnectorHandler(EventLoop *loop);
	~SocketConnectorHandler() override;

	/**
	 * @brief Try to connect to peer, if another connect in progress, do nothing
	 */
	void connect(const Address &address);

	/**
	 * @brief If connecting, stop connect, otherwise do nothing.
	 */
	void stopConnect();

	/**
	 * @brief Get the SocketConnectionHandler connected by SocketConnectorHandler
	 */
	std::weak_ptr<SocketConnectionHandler> getConnection() const { return _connection; }

	/**
	 * @brief Is already connected
	 */
	bool connected() const { return m_connectionEstablished.load(std::memory_order_relaxed); }

protected:
	/**
	 * @brief Handle EPOLLOUT triggered when connect state changed,
	 * if connect failed in some situations, should start retry connect.
	 * @note Handlers will run only in EventLoop, NOT thread safe.
	 *
	 */
	void handleOut() override;

	int fileDescriptor() const override;

private:
	/**
	 * @brief Try to reconnect later
	 * 
	 */
	void reconnect();

	std::weak_ptr<SocketConnectionHandler> _connection;

	std::unique_ptr<SocketDevice> m_socket;
	std::unique_ptr<Timer> m_retryTimer;

	Address m_address;

	ConnectedCallBack m_connectedCallback;
	ErrorCallBack m_errorCallback;

	/**
	 * @brief the state of connector
	 *           stop()   retry()
	 *           +--<-+   +-<-+
	 *           |    |   |   |
	 * * -> Closed -> Connecting -> Established
	 *  make     connect      connected
	 */
	TcpState m_state;
	std::atomic_bool m_connectionEstablished;
};
}

#endif
