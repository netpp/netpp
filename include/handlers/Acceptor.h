#ifndef NETPP_ACCEPTOR_H
#define NETPP_ACCEPTOR_H

#include "epoll/EventHandler.h"
#include "socket/Socket.h"
#include "Events.h"
#include "Address.h"

namespace netpp {
class EventLoopDispatcher;
}

namespace netpp::handlers {
// TODO: close acceptor
class Acceptor : public epoll::EventHandler {
public:
	Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket) noexcept;
	~Acceptor() override = default;

	void listen() noexcept;

	void handleRead() noexcept override;
	void handleWrite() noexcept override;
	void handleError() noexcept override;
	void handleClose() noexcept override;
	static bool makeAcceptor(EventLoopDispatcher *dispatcher,
								 Address listenAddr,
								 std::unique_ptr<support::EventInterface> &&eventsPrototype) noexcept;

private:
	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<socket::Socket> m_socket;
	std::unique_ptr<support::EventInterface> m_events;
};
}

#endif