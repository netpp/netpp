#ifndef NETPP_SIGNAL_HANDLER_H
#define NETPP_SIGNAL_HANDLER_H

#include "../epoll/EventHandler.h"
#include "Events.h"

namespace netpp {
class EventLoop;
}

namespace netpp::handlers {
class SignalHandler : public epoll::EventHandler, public std::enable_shared_from_this<SignalHandler> {
public:
	SignalHandler() = default;
	~SignalHandler() override = default;

	void handleRead() override;
	void handleWrite() override {};
	void handleError() override {};
	void handleClose() override;

	static std::shared_ptr<SignalHandler> makeSignalHandler(EventLoop *loop, Events eventsPrototype);
private:
	Events m_events;
};
}

#endif
