#ifndef NETPP_ACCEPTOR_H
#define NETPP_ACCEPTOR_H

#include "epoll/EventHandler.h"
#include "support/Socket.h"
#include "Events.h"
#include "Address.h"

namespace netpp {
class EventLoopDispatcher;
}

namespace netpp::handlers {
// TODO: close acceptor
class Acceptor : public epoll::EventHandler {
public:
	Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<support::Socket> &&socket);
	~Acceptor() override = default;

	void listen();

	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleClose() override;
	static bool makeAcceptor(EventLoopDispatcher *dispatcher,
								 Address listenAddr,
								 std::unique_ptr<support::EventInterface> &&eventsPrototype);

private:
	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<support::Socket> m_socket;
	std::unique_ptr<support::EventInterface> m_events;
};
}

#endif