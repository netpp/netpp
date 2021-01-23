#ifndef NETPP_CONNECTOR_H
#define NETPP_CONNECTOR_H

#include <memory>
#include "epoll/EventHandler.h"
#include "socket/Socket.h"
#include "Timer.h"
#include "Events.h"
#include "Address.h"

namespace netpp {
class EventLoopDispatcher;
}

namespace netpp::handlers {
// TODO: close connector
class Connector : public epoll::EventHandler {
public:
	Connector(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket) noexcept;
	~Connector() override = default;

	void connect() noexcept;
	void handleRead() noexcept override;
	void handleWrite() noexcept override;
	void handleError() noexcept override;
	void handleClose() noexcept override;

	static bool makeConnector(EventLoopDispatcher *dispatcher,
								  Address serverAddr,
								  std::unique_ptr<support::EventInterface> &&eventsPrototype) noexcept;

private:
	void reconnect() noexcept;

	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<socket::Socket> m_socket;
	std::unique_ptr<Timer> m_retryTimer;
	std::unique_ptr<support::EventInterface> m_events;
};
}

#endif
