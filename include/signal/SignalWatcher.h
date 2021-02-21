#ifndef NETPP_SIGNAL_WATCHER_H
#define NETPP_SIGNAL_WATCHER_H

#include "./Signals.h"
#include "Events.h"
#include <thread>

namespace netpp {
namespace internal::handlers {
class SignalHandler;
}
class EventLoopDispatcher;
}

namespace netpp::signal {
class SignalWatcher {
	// make sure only SignalHandler can access signal fd
	friend class internal::handlers::SignalHandler;
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

	static ::sigset_t *m_watchingSignals;
	static std::shared_ptr<internal::handlers::SignalHandler> m_signalHandler;

	static std::thread m_unHandledSignalThread;
	static std::mutex m_watchSignalMutex;
	static std::condition_variable m_waitWatchSignalChange;
};
}

#endif
