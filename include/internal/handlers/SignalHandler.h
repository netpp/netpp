#ifndef NETPP_SIGNAL_HANDLER_H
#define NETPP_SIGNAL_HANDLER_H

#include "internal/epoll/EventHandler.h"
#include "Events.h"

namespace netpp::eventloop {
class EventLoop;
}

namespace netpp::signal {
enum class Signals;
}

namespace netpp::internal::handlers {
/**
 * @brief The SignalHandler handles watched signals, and notify user
 * 
 */
class SignalHandler : public epoll::EventHandler, public std::enable_shared_from_this<SignalHandler> {
public:
	/// @brief Use makeSignalHandler to create SignalHandler
	explicit SignalHandler(eventloop::EventLoop *loop);
	~SignalHandler() override;

	void startWatchSignal(signal::Signals signal);
	void stopWatchSignal(signal::Signals signal);

	/**
	 * @brief Create a signal handler, thread safe
	 * @note There should be ONLY ONE signal handler in system
	 * 
	 * @param loop								The EventLoop signal handler lives in
	 * @param eventsPrototype					User-define signal handler
	 * @return std::shared_ptr<SignalHandler>	SignalHandler created
	 */
	static std::shared_ptr<SignalHandler> makeSignalHandler(eventloop::EventLoop *loop, Events eventsPrototype);

protected:
	/**
	 * @brief Handle read events on signal fd, triggered when
	 * 1.signal in watching set emits
	 * @note Handlers will run only in EventLoop, NOT thread safe
	 *
	 */
	void handleIn() override;

private:
	int m_signalFd;
	Events m_events;
	::sigset_t *m_watchingSignals;
};
}

#endif
