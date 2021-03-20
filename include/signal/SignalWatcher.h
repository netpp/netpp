#ifndef NETPP_SIGNAL_WATCHER_H
#define NETPP_SIGNAL_WATCHER_H

#include <mutex>
#include <memory>
#include <thread>
#include <condition_variable>

namespace netpp {
namespace internal::handlers {
class SignalHandler;
}
class EventLoopDispatcher;
class Events;
}

namespace netpp::signal {
enum class Signals;
/**
 * @brief The signal handle system, use signal fd
 * 
 */
class SignalWatcher {
	// make sure only SignalHandler can access signal fd
	friend class internal::handlers::SignalHandler;
public:
	SignalWatcher() = default;

	/**
	 * @brief enable signal handling
	 * @note must call before any threads started, or signal may send to unexpected thread,
	 * recommending use this at the beginning of main()
	 * 
	 */
	static void enableWatchSignal();

	/**
	 * @brief init signal event handler
	 * 
	 * @param dispatcher		event loop dispatcher
	 * @param eventsPrototype	how to handle signal event
	 */
	static SignalWatcher with(EventLoopDispatcher *dispatcher, Events eventsPrototype);

	/// @brief start watch signal, take over default signal action
	static SignalWatcher watch(Signals signal);

	/// @brief stop watch signal, handle it by default
	static SignalWatcher restore(Signals signal);

	/// @brief is watching signal
	static bool isWatching(Signals signal);
	static bool isWatching(int signal);

// for SignalHandler
private:
	static int signalFd;

private:
	static ::sigset_t *m_watchingSignals;
	static std::shared_ptr<internal::handlers::SignalHandler> m_signalHandler;

	static std::thread m_unHandledSignalThread;
	static std::mutex m_watchSignalMutex;
	/// @brief notify signal watch set changed
	static std::condition_variable m_waitWatchSignalChange;
};
}

#endif
