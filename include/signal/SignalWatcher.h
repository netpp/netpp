#ifndef NETPP_SIGNAL_WATCHER_H
#define NETPP_SIGNAL_WATCHER_H

#include "./Signals.h"
#include <atomic>
#include "support/EventsInterface.h"

namespace netpp {
namespace handlers {
class SignalHandler;
}
class EventLoop;
}

namespace netpp::signal {
class SignalWatcher {
	// make sure only SignalHandler can access signal fd
	friend class handlers::SignalHandler;
public:
	SignalWatcher() = default;

	static void watch(Signals signal);
	static void restore(Signals signal);
	static bool isWatching(Signals signal);
	static bool isWatching(uint32_t signal);

	/**
	 * @brief enable signal handlering
	 * @note must call before any threads started
	 * 
	 */
	static void enableWatchSignal(EventLoop *loop, std::unique_ptr<support::EventInterface> &&eventsPrototype);

private:
	static int signalFd;
	// 64 signals at max
	static volatile std::atomic_uint64_t m_watchingSignals;
};
}

#endif
