#ifndef NETPP_TCP_CONNECTOR_H
#define NETPP_TCP_CONNECTOR_H

#include <memory>
#include "epoll/EventHandler.h"
#include "socket/Socket.h"
#include "ByteArray.h"
#include "Events.h"

namespace netpp {
class EventLoop;
namespace internal::socket {
enum class TcpState;
}
namespace time {
class TimeWheelEntry;
}
}

namespace netpp::internal::handlers {
class TcpConnection : public epoll::EventHandler, public std::enable_shared_from_this<TcpConnection> {
public:
	explicit TcpConnection(std::unique_ptr<socket::Socket> &&socket, EventLoop *loop);
	~TcpConnection() override = default;

	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleClose() override;

	void sendInLoop();
	void closeAfterWriteCompleted();
	std::shared_ptr<Channel> getIOChannel();

	static std::weak_ptr<TcpConnection> makeTcpConnection(EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
									  Events eventsPrototype);

private:
	void renewWheel();
	void closeWrite();

	EventLoop *_loop;
	internal::socket::TcpState m_state;
	bool m_isWaitWriting;
	std::unique_ptr<socket::Socket> m_socket;
	std::shared_ptr<ByteArray> m_writeBuffer;
	std::shared_ptr<ByteArray> m_receiveBuffer;
	Events m_events;

	std::weak_ptr<time::TimeWheelEntry> _idleConnectionWheel;
	std::weak_ptr<time::TimeWheelEntry> _halfCloseWheel;
};
}

#endif
