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
class Acceptor : public epoll::EventHandler, public std::enable_shared_from_this<Acceptor> {
public:
	Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket);
	~Acceptor() override = default;

	void listen();

	/**
	 * @brief stop accept incomming connections, remove Acceptor from EventLoop
	 */
	void stop();

	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleClose() override;
	static std::weak_ptr<Acceptor> makeAcceptor(EventLoopDispatcher *dispatcher,
								 Address listenAddr,
								 std::unique_ptr<support::EventInterface> &&eventsPrototype);

private:
	Address m_addr;

	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<socket::Socket> m_socket;
	std::unique_ptr<support::EventInterface> m_events;
};
}

#endif