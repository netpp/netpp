#ifndef NETPP_SIGNAL_WATCHER_H
#define NETPP_SIGNAL_WATCHER_H

#include "./Signals.h"
#include <atomic>
#include "Events.h"

namespace netpp {
namespace handlers {
class SignalHandler;
}
class EventLoopDispatcher;
}

namespace netpp::signal {
class SignalWatcher {
	// make sure only SignalHandler can access signal fd
	friend class handlers::SignalHandler;
public:
	SignalWatcher() = default;

	/**
	 * @brief init signal event handler
	 * 
	 * @param dispatcher		event loop dispatcher
	 * @param eventsPrototype	how to handle signal event
	 */
	static SignalWatcher with(EventLoopDispatcher *dispatcher, Events eventsPrototype);
	static SignalWatcher watch(Signals signal);
	static SignalWatcher restore(Signals signal);
	static bool isWatching(Signals signal);
	static bool isWatching(uint32_t signal);

	/**
	 * @brief enable signal handlering
	 * @note must call before any threads started, or signal may send to unexpected thread
	 * 
	 */
	static void enableWatchSignal();

private:
	static int signalFd;
	// 64 signals at max
	static volatile std::atomic_uint64_t m_watchingSignals;
	static std::shared_ptr<handlers::SignalHandler> m_signalHandler;
};
}

#endif
