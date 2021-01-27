#ifndef NETPP_CONNECTOR_H
#define NETPP_CONNECTOR_H

#include <memory>
#include "epoll/EventHandler.h"
#include "socket/Socket.h"
#include "time/Timer.h"
#include "Events.h"
#include "Address.h"

namespace netpp {
class EventLoopDispatcher;
}

namespace netpp::handlers {
// TODO: close connector
class Connector : public epoll::EventHandler, public std::enable_shared_from_this<Connector> {
public:
	Connector(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket);
	~Connector() override = default;

	void connect();
	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleClose() override;

	static bool makeConnector(EventLoopDispatcher *dispatcher,
								  Address serverAddr,
								  std::unique_ptr<support::EventInterface> &&eventsPrototype);

private:
	void setupTimer();
	void reconnect();

	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<socket::Socket> m_socket;
	std::unique_ptr<time::Timer> m_retryTimer;
	std::unique_ptr<support::EventInterface> m_events;
};
}

#endif
