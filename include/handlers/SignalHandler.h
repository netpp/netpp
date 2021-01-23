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
	SignalHandler() noexcept;
	~SignalHandler() override = default;

	void handleRead() noexcept override;
	void handleWrite() noexcept override {};
	void handleError() noexcept override {};
	void handleClose() noexcept override {};

	static void makeSignalHandler(EventLoop *loop, std::unique_ptr<support::EventInterface> &&eventsPrototype) noexcept;
private:
	std::unique_ptr<support::EventInterface> m_events;
};
}

#endif
