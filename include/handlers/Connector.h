#ifndef NETPP_CONNECTOR_H
#define NETPP_CONNECTOR_H

#include <memory>
#include "epoll/EventHandler.h"
#include "Socket.h"
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
	Connector(EventLoopDispatcher *dispatcher, std::unique_ptr<Socket> &&socket);
	~Connector() override = default;

	void connect();
	void handleRead() override;
	void handleWrite() override;
	void handleError() override;
	void handleClose() override;

	static bool makeConnector(EventLoopDispatcher *dispatcher,
								  Address serverAddr,
								  std::unique_ptr<Events> &&eventsPrototype);

private:
	void reconnect();

	EventLoopDispatcher *_dispatcher;
	std::unique_ptr<Socket> m_socket;
	std::unique_ptr<Timer> m_retryTimer;
	std::unique_ptr<Events> m_events;
};
}

#endif
