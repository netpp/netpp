#ifndef NETPP_TCP_CONNECTOR_H
#define NETPP_TCP_CONNECTOR_H

#include <memory>
#include "internal/epoll/EventHandler.h"
#include "Events.h"
#include <atomic>

namespace netpp {
class EventLoop;
class ByteArray;
namespace internal::socket {
enum class TcpState;
class Socket;
}
namespace internal::time {
class TimeWheelEntry;
}
}

namespace netpp::internal::handlers {
/**
 * @brief The TcpConnection represent a connection, with read/write @see ByteArray,
 * use a @see Channel to perform read/write to buffer.
 * 
 */
class TcpConnection : public epoll::EventHandler, public std::enable_shared_from_this<TcpConnection> {
	friend class HalfCloseConnectionWheelEntry;
public:
	/// @brief Use makeTcpConnection to create TcpConnection
	explicit TcpConnection(std::unique_ptr<socket::Socket> &&socket);
	~TcpConnection() override = default;

	/**
	 * @brief After write some data to ByteArray, use this to send.
	 * send operation will not effect immediately, should be sent later.
	 * thread safe.
	 */
	void sendInLoop();

	/**
	 * @brief Close this connection after all pending data is send.
	 * close operation will not effect immediately, should be closed later.
	 * thread safe.
	 * 
	 */
	void closeAfterWriteCompleted();

	/**
	 * @brief Get the Channel read/write to connection buffer
	 * thread safe
	 * @return std::shared_ptr<Channel>	the read/write channel to this connection
	 */
	std::shared_ptr<Channel> getIOChannel();

	internal::socket::TcpState currentState() { return m_state.load(std::memory_order_acquire); }

	/**
	 * @brief Get the unique connection id
	 */
	int connectionId();

	static std::weak_ptr<TcpConnection> makeTcpConnection(EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
									  Events eventsPrototype);

protected:
	/**
	 * @brief handle read events on connection, triggered when
	 * 1.socket has pending read data
	 * @note handlers will run only in EventLoop, NOT thread safe
	 */
	void handleIn() override;

	/**
	 * @brief handle write events on connection, triggered when
	 * 1.socket is ready to write
	 * @note handlers will run only in EventLoop, NOT thread safe
	 */
	void handleOut() override;

	/**
	 * @brief handle error events on connection, triggered when
	 * 1.side close/shutdown write on this connection
	 * @note handlers will run only in EventLoop, NOT thread safe
	 */
	void handleRdhup() override;

private:
	/**
	 * @brief Refresh time wheel when received any data, prevent closed by
	 * time wheel
	 * Not thread safe
	 */
	void renewWheel();

	/**
	 * @brief Close write on this side, enter tcp four-way-wavehand step,
	 * if side not close connection and no data transferred, the connection
	 * will be force closed
	 * Not thread safe
	 */
	void closeWrite();

	/**
	 * @brief Force close current connection
	 * Not thread safe
	 */
	void forceClose();

	/**
	 * @brief The state for TcpConnection
	 *              idle      halfCloseTimeout
	 *           +---->----+    +-->---+
	 *           |         |    |      |
	 * * -> Established -> Closing -> Closed
	 *  make       closeWrite()  handleClose()
	 */
	std::atomic<internal::socket::TcpState> m_state;

	/// @brief if any data wait for write, the closing should be done later
	bool m_isWaitWriting;
	std::unique_ptr<socket::Socket> m_socket;
	/// @brief pending write data
	std::shared_ptr<ByteArray> m_writeBuffer;
	/// @brief received data
	std::shared_ptr<ByteArray> m_receiveBuffer;
	Events m_events;

	/// @brief if connection is idle, closed by this time wheel
	std::weak_ptr<internal::time::TimeWheelEntry> _idleConnectionWheel;
	/// @brief if connection is half close, and no data transferred, closed by this time wheel
	std::weak_ptr<internal::time::TimeWheelEntry> _halfCloseWheel;
};
}

#endif
