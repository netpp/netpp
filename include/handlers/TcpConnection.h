#ifndef NETPP_TCP_CONNECTOR_H
#define NETPP_TCP_CONNECTOR_H

#include <memory>
#include "epoll/EventHandler.h"
#include "socket/Socket.h"
#include "ByteArray.h"
#include "Events.h"

namespace netpp {
class EventLoop;
namespace socket {
enum class TcpState;
}
namespace time {
class TimeWheelEntry;
}
}

namespace netpp::handlers {
class TcpConnection : public epoll::EventHandler, public std::enable_shared_from_this<TcpConnection> {
public:
	explicit TcpConnection(std::unique_ptr<socket::Socket> &&socket, EventLoop *loop);
	~TcpConnection() override = default;

	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleClose() override;

	inline EventLoop *getConnectionLoop() { return _loop; }
	void sendInLoop();
	void closeAfterWriteCompleted();
	std::shared_ptr<Channel> getIOChannel();

	static std::weak_ptr<TcpConnection> makeTcpConnection(EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
									  std::unique_ptr<support::EventInterface> &&eventsPrototype);

private:
	void renewWheel();
	void closeWrite();

	EventLoop *_loop;
	socket::TcpState m_state;
	bool m_isWaitWriting;
	std::unique_ptr<socket::Socket> m_socket;
	std::shared_ptr<ByteArray> m_writeBuffer;
	std::shared_ptr<ByteArray> m_receiveBuffer;
	std::unique_ptr<support::EventInterface> m_events;

	std::weak_ptr<time::TimeWheelEntry> _idleConnectionWheel;
	std::weak_ptr<time::TimeWheelEntry> _halfCloseWheel;
};
}

#endif
