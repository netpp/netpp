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
}

namespace netpp::handlers {
// TODO: kick idle connections
class TcpConnection : public epoll::EventHandler, public std::enable_shared_from_this<TcpConnection> {
public:
	explicit TcpConnection(std::unique_ptr<socket::Socket> &&socket, EventLoop *loop) noexcept;
	~TcpConnection() override = default;

	void handleRead() noexcept override;
	void handleWrite() noexcept override;
	void handleError() noexcept override;
	void handleClose() noexcept override;

	inline EventLoop *getConnectionLoop() noexcept { return _loop; }
	void sendInLoop() noexcept;
	void closeAfterWriteCompleted() noexcept;

	static std::shared_ptr<Channel> makeTcpConnection(EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
									  std::unique_ptr<support::EventInterface> &&eventsPrototype) noexcept;

private:
	EventLoop *_loop;
	socket::TcpState m_state;
	bool m_isWaitWriting;
	std::unique_ptr<socket::Socket> m_socket;
	std::shared_ptr<ByteArray> m_writeBuffer;
	std::shared_ptr<ByteArray> m_receiveBuffer;
	std::unique_ptr<support::EventInterface> m_events;
};
}

#endif
