#ifndef NETPP_SIGNAL_HANDLER_H
#define NETPP_SIGNAL_HANDLER_H

#include "internal/epoll/EventHandler.h"
#include "Events.h"

namespace netpp {
class EventLoop;
}

namespace netpp::internal::handlers {
/**
 * @brief The SignalHandler handles watched signals, and notify user
 * 
 */
class SignalHandler : public epoll::EventHandler, public std::enable_shared_from_this<SignalHandler> {
public:
	/// @brief Use makeSignalHandler to create SignalHandler
	SignalHandler() = default;
	~SignalHandler() override = default;

	/**
	 * @brief Stop handle signal fd event, however, watched signal emited will still
	 * send to signal fd
	 * 
	 */
	void stop();

	/**
	 * @brief Create a signal handler, thread safe
	 * @note There should be ONLY ONE signal handler in system
	 * 
	 * @param loop								The EventLoop signal handler lives in
	 * @param eventsPrototype					User-define signal handler
	 * @return std::shared_ptr<SignalHandler>	SignalHandler created
	 */
	static std::shared_ptr<SignalHandler> makeSignalHandler(EventLoop *loop, Events eventsPrototype);

protected:
	/**
	 * @brief Handle read events on signal fd, triggered when
	 * 1.signal in watching set emits
	 * @note Handlers will run only in EventLoop, NOT thread safe
	 *
	 */
	void handleIn() override;

private:
	Events m_events;
};
}

#endif
