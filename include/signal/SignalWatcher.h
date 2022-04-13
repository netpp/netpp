#ifndef NETPP_SIGNAL_WATCHER_H
#define NETPP_SIGNAL_WATCHER_H

namespace netpp {
class Application;
}

namespace netpp::signal {
enum class Signals;
/**
 * @brief Take care of signals emits in event loop
 * @note By default, thread signal masks inherit creating thread, and the SignalWatcher
 * handles only signals on event loop. For threads create by you, the following result may
 * happen:
 * 1. No signal send to your threads, because all signals were blocked at initialization of netpp.
 * 
 */
class SignalWatcher {
	friend class netpp::Application;
public:
	/**
	 * @brief start watch signal, take over default signal action
	 * @note set start watch a signal will not effect immediately due to event loop's schedule
	 */
	static void watch(Signals signal);

	/** @brief stop watch signal, handle it by default
	 * @note set stop watch a signal will not effect immediately due to event loop's schedule
	 */
	static void restore(Signals signal);

	/**
	 * @brief is watching signal
	 * @param signal netpp signal enumerator
	 */
	static bool isWatching(Signals signal);

	/**
	 * @brief is watching signal
	 * @param signal linux signal number
	 */
	static bool isWatching(int signal);

private:
	static void enableWatchSignal();
};
}

#endif
