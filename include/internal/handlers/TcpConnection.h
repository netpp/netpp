#ifndef NETPP_TCP_CONNECTOR_H
#define NETPP_TCP_CONNECTOR_H

#include <memory>
#include "internal/epoll/EventHandler.h"
#include "Events.h"
#include "time/TimerType.h"
#include <atomic>
#include "Config.h"

namespace netpp {
namespace eventloop {
class EventLoop;
}
class ByteArray;
namespace time {
class TickTimer;
}
namespace internal {
namespace socket {
enum class TcpState;
class Socket;
}
namespace buffer {
class ChannelBufferConversion;
}
}
}

namespace netpp::internal::handlers {
/**
 * @brief The TcpConnection class represent a connection, using @see ByteArray as buffer,
 * provide an io @see Channel to read/write.
 * @note This class lives in event loop, only public methods are thread safe
 * 
 */
class TcpConnection : public epoll::EventHandler, public std::enable_shared_from_this<TcpConnection> {
public:
	/// @brief Use makeTcpConnection to create TcpConnection
	TcpConnection(eventloop::EventLoop *loop, std::unique_ptr<socket::Socket> &&socket);
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

	internal::socket::TcpState currentState() const { return m_state.load(std::memory_order_acquire); }

	/**
	 * @brief Get the unique connection id
	 * thread safe
	 */
	int connectionId();

	static std::shared_ptr<TcpConnection> makeTcpConnection(eventloop::EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
															Events eventsPrototype, ConnectionConfig config);

protected:
	/**
	 * @brief handle read events on connection, triggered when
	 * 1.socket has pending read data
	 * @note NOT thread safe, handlers will run only in EventLoop
	 */
	void handleIn() override;

	/**
	 * @brief handle write events on connection, triggered when
	 * 1.socket is ready to write
	 * @note NOT thread safe, handlers will run only in EventLoop
	 */
	void handleOut() override;

	/**
	 * @brief handle error events on connection, triggered when
	 * 1.side close/shutdown write on this connection
	 * @note NOT thread safe, handlers will run only in EventLoop
	 */
	void handleRdhup() override;

private:
	/**
	 * @brief Refresh time wheel when received any data, prevent closed by
	 * time wheel
	 * @note Not thread safe
	 */
	void renewWheel();

	/**
	 * @brief Force close this connection
	 * @note Not thread safe
	 */
	void forceClose();

	/**
	 * @brief The state transition of TcpConnection
	 *
	 *                      +------------>------------+    +------------>------------+
	 *                      |         (idle)          |    |   (half close timeout)  |
	 *                      |                         |    |                         |
	 * * -------> Established ---------->---------- HalfClose ----------->--------- Closed --> *
	 * (construct)        | |    (active shutdown)    |    |   (write completed)     |
	 *                    | |                         |    |                         |
	 *                    | +------------->-----------+    |                         |
	 *                    |      (pear shutdown)           |                         |
	 *                    |                                |                         |
	 *                    +----------------------------->--+-------------------------+
	 *                                              (io error)
	 */
	std::atomic<internal::socket::TcpState> m_state;

	/// @brief If any data wait for writing, close operation should be done later
	bool m_isWaitWriting;
	std::unique_ptr<socket::Socket> m_socket;
	/// @brief Buffer for prepend write data
	std::shared_ptr<Channel> m_connectionBufferChannel;
	std::unique_ptr<buffer::ChannelBufferConversion> m_bufferConvertor;
	/// @brief User-defined event handler
	Events m_events;

	ConnectionConfig m_config;
	/// @brief if connection is idle, closed by this time wheel
	std::unique_ptr<time::TickTimer> m_idleConnectionWheel;
	/// @brief if connection is half close, and no data transferred, closed by this time wheel
	std::unique_ptr<time::TickTimer> m_halfCloseWheel;
};
}

#endif
