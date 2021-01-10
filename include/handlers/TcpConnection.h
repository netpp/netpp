#ifndef NETPP_TCP_CONNECTOR_H
#define NETPP_TCP_CONNECTOR_H

#include <memory>
#include "epoll/EventHandler.h"
#include "Socket.h"
#include "ByteArray.h"
#include "Events.h"

namespace netpp {
class EventLoop;
}

namespace netpp::handlers {
enum class TcpState {
	Connecting, Connected, Disconnecting, Disconnected
};
class TcpConnection : public epoll::EventHandler, public std::enable_shared_from_this<TcpConnection> {
public:
	explicit TcpConnection(std::unique_ptr <Socket> &&socket, EventLoop *loop);
	~TcpConnection() override = default;

	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleClose() override;

	inline EventLoop *getConnectionLoop() { return _loop; }
	void sendInLoop();
	void closeAfterWriteCompleted();

	static std::shared_ptr<Channel> makeTcpConnection(EventLoop *loop, std::unique_ptr<Socket> &&socket,
									  std::unique_ptr<Events> &&eventsPrototype);

private:
	EventLoop *_loop;
	TcpState m_state;
	bool m_isWaitWriting;
	std::unique_ptr<Socket> m_socket;
	std::shared_ptr<ByteArray> m_writeBuffer;
	std::shared_ptr<ByteArray> m_receiveBuffer;
	std::unique_ptr<Events> m_events;
};
}

#endif
