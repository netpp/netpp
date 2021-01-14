#ifndef NETPP_SINGAL_HANDLER_H
#define NETPP_SIGNAL_HANDLER_H

#include "../epoll/EventHandler.h"
#include "Events.h"

namespace netpp {
class EventLoop;
}

namespace netpp::handlers {
class SignalHandler : public epoll::EventHandler {
public:
	SignalHandler();
	~SignalHandler() override = default;

	void handleRead() override;
	void handleWrite() override {};
	void handleError() override {};
	void handleClose() override {};

	static void makeSignalHandler(EventLoop *loop, std::unique_ptr<support::EventInterface> &&eventsPrototype);
private:
	std::unique_ptr<support::EventInterface> m_events;
};
}

#endif
