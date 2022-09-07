#ifndef NETPP_TCP_CONNECTOR_H
#define NETPP_TCP_CONNECTOR_H

#include <memory>
#include <atomic>
#include "support/Types.h"
#include "epoll/EpollEventHandler.h"

namespace netpp {
class EventLoop;
class ByteArray;
class TickTimer;
enum class TcpState;
class SocketDevice;
class TransferBuffer;
/**
 * @brief The SocketConnectionHandler class represent a connection, using @see ByteArray as buffer,
 * provide an io @see Channel to read/write.
 * @note This class lives in event loop, only public methods are thread safe
 * 
 */
class SocketConnectionHandler : public EpollEventHandler, public std::enable_shared_from_this<SocketConnectionHandler> {
public:
	/**
	 * @brief
	 * @param loop The event loop this handler lives
	 * @param socket The socket to be manage
	 * @param idleTime Specify how long the socket keeps open if no data being transferred, pass -1 if do not need auto close connection, default -1
	 * @param halfCloseTime Specify how long the socket lives before force closed in half close state if no data being transferred, pass -1 if do not need auto close connection, default -1
	 */
	SocketConnectionHandler(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket,
							std::shared_ptr<Channel> channelToBind, std::shared_ptr<TransferBuffer> buffer);
	~SocketConnectionHandler() override;

	void setIdleTimeout(TimerInterval idleTime);

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
	 * @brief Force close this connection.
	 * Note that it does not close immediately, rather closed in event loop.
	 * Thread safe.
	 */
	void forceClose();

	/**
	 * @brief Get the Channel read/write to connection buffer
	 * thread safe
	 * @return std::shared_ptr<Channel>	the read/write channel to this connection
	 */
	std::shared_ptr<Channel> getIOChannel();

	TcpState currentState() const { return m_state.load(std::memory_order_acquire); }

	void setMessageReceivedCallBack(const MessageReceivedCallBack &cb);
	void setWriteCompletedCallBack(const WriteCompletedCallBack &cb);
	void setDisconnectedCallBack(const DisconnectedCallBack &cb);
	void setErrorCallBack(const ErrorCallBack &cb);

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

	int fileDescriptor() const override;

private:
	/**
	 * @brief Refresh time wheel when received any data, prevent closed by
	 * time wheel
	 * @note Not thread safe
	 */
	void renewWheel();

	/**
	 * @brief The state transition of SocketConnectionHandler
	 *
	 *                      +------------>------------+    +------------>------------+
	 *                      |         (idle)          |    |   (half close timeout)  |
	 *                      |                         |    |                         |
	 * * -------> Established ---------->---------- HalfClose ----------->--------- Closed --> *
	 * (construct)        | |    (activeEvents shutdown)    |    |   (write completed)     |
	 *                    | |                         |    |                         |
	 *                    | +------------->-----------+    |                         |
	 *                    |      (pear shutdown)           |                         |
	 *                    |                                |                         |
	 *                    +----------------------------->--+-------------------------+
	 *                                              (io error)
	 */
	std::atomic<TcpState> m_state;

	/// @brief If any data wait for writing, close operation should be done later
	bool m_isWaitWriting;
	std::unique_ptr<SocketDevice> m_socket;
	std::shared_ptr<Channel> m_bindChannel;
	/// @brief TransferBuffer for prepend write data
	std::shared_ptr<TransferBuffer> m_connectionBuffer;

	MessageReceivedCallBack m_receivedCallback;
	WriteCompletedCallBack m_writeCompletedCallback;
	DisconnectedCallBack m_disconnectedCallback;
	ErrorCallBack m_errorCallback;

	TimerInterval m_idleTimeInterval;
	/// @brief if connection is idle, closed by this time wheel
	std::unique_ptr<TickTimer> m_idleConnectionWheel;
	/// @brief if connection is half close, and no data transferred, closed by this time wheel
	std::unique_ptr<TickTimer> m_halfCloseWheel;
};
}

#endif
