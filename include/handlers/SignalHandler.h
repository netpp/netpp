#ifndef NETPP_SIGNAL_HANDLER_H
#define NETPP_SIGNAL_HANDLER_H

#include "epoll/EventHandler.h"
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
	 * @brief handle read events on signal fd, triggered when
	 * 1.signal in watching set emits
	 * @note handlers will run only in EventLoop, NOT thread safe
	 * 
	 */
	void handleIn() override;

	/**
	 * @brief stop handle signal fd event, however, watched signal emited will still
	 * send to signal fd
	 * 
	 */
	void stop();

	/**
	 * @brief create a signal handler, thread safe
	 * @note there should be ONLY ONE signal handler in system
	 * 
	 * @param loop								the EventLoop signal handler lives in
	 * @param eventsPrototype					user-define signal handler
	 * @return std::shared_ptr<SignalHandler>	SignlaHandler created
	 */
	static std::shared_ptr<SignalHandler> makeSignalHandler(EventLoop *loop, Events eventsPrototype);

private:
	Events m_events;
};
}

#endif
