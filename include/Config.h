#ifndef NETPP_CONFIG_H
#define NETPP_CONFIG_H

#include "Events.h"
#include "time/TimerType.h"

namespace netpp {
struct ConnectionConfig {
	/** @brief Enable time wheel to close idle connection */
	bool enableAutoClose;
	time::TimerInterval idleConnectionTimeout;
	time::TimerInterval halfCloseConnectionTimeout;
	// enable retry connect, max retry count
};

/**
 * @brief The global m_config for netpp application
 */
struct Config {
	/** @brief Event loop number */
	unsigned eventLoopNumber;
// event loop wait interval
//	/** @brief Enable run user-defined handler in event loop */
//	bool enableRunHandlerInThread = false;
	// user-defined handler threads number
	/** @brief enable log */
	bool enableLog;
	struct {
		/** @brief Enable TickTimer */
		bool enable;
		/** @brief Wheel tick interval, in second */
		time::TimerInterval tickInterval;
		/** @brief The size of Wheel */
		time::TimerInterval maxLength;
	} tickTimer;
	struct SignalConfig {
		/** @brief Enable handle signal */
		bool enableHandleSignal;
		std::vector<netpp::signal::Signals> interestedSignals;

	} signal;
	/** @brief Support dns resolve */
	bool enableDnsResolve;
	/** @brief The global event handler */
	netpp::Events eventHandler;
	ConnectionConfig connection;

	Config()
		: eventLoopNumber{1}, enableLog{false},
		  tickTimer{true, 1000, 600},
		  signal{},
		  enableDnsResolve{true},
		  eventHandler{}, connection{false, 60000, 60000}
	{}
	Config(const Config &other)
		: signal(other.signal)
	{
		eventLoopNumber = other.eventLoopNumber;
		enableLog = other.enableLog;
		tickTimer.enable = other.tickTimer.enable;
		tickTimer.tickInterval = other.tickTimer.tickInterval;
		tickTimer.maxLength = other.tickTimer.maxLength;
		enableDnsResolve = other.enableDnsResolve;
		eventHandler = other.eventHandler;
		connection.enableAutoClose = other.connection.enableAutoClose;
		connection.halfCloseConnectionTimeout = other.connection.halfCloseConnectionTimeout;
		connection.idleConnectionTimeout = other.connection.idleConnectionTimeout;
	}
	Config(Config &&other) noexcept
			: signal(std::move(other.signal))
	{
		eventLoopNumber = other.eventLoopNumber;
		enableLog = other.enableLog;
		tickTimer.enable = other.tickTimer.enable;
		tickTimer.tickInterval = other.tickTimer.tickInterval;
		tickTimer.maxLength = other.tickTimer.maxLength;
		enableDnsResolve = other.enableDnsResolve;
		eventHandler = other.eventHandler;
		connection.enableAutoClose = other.connection.enableAutoClose;
		connection.halfCloseConnectionTimeout = other.connection.halfCloseConnectionTimeout;
		connection.idleConnectionTimeout = other.connection.idleConnectionTimeout;
	}

// buffer node size
};
}

#endif
