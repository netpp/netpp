#ifndef NETPP_SIGNAL_HANDLER_H
#define NETPP_SIGNAL_HANDLER_H

#include "epoll/EpollEventHandler.h"
#include "Events.h"

namespace netpp {
class EventLoop;
enum class Signals;
/**
 * @brief The SignalHandler handles watched signals, and notify user
 * 
 */
class SignalHandler : public EpollEventHandler, public std::enable_shared_from_this<SignalHandler> {
public:
	/// @brief Use makeSignalHandler to create SignalHandler
	explicit SignalHandler(EventLoop *loop, Events eventsPrototype, const std::vector<netpp::Signals> &interestedSignals);
	~SignalHandler() override;

protected:
	/**
	 * @brief Handle read events on signal fd, triggered when
	 * 1.signal in watching set emits
	 * @note Handlers will run only in EventLoop, NOT thread safe
	 *
	 */
	void handleIn() override;

	int fileDescriptor() const override { return m_signalFd; }

private:
	int m_signalFd;
	Events m_events;
};
}

#endif
