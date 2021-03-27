#ifndef NETPP_SIGNAL_WATCHER_H
#define NETPP_SIGNAL_WATCHER_H

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
};
}

#endif
