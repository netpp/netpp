#ifndef NETPP_CONFIG_H
#define NETPP_CONFIG_H

#include "Events.h"
#include "time/TimerType.h"

namespace netpp {
struct ConnectionConfig {
	/** @brief Enable time wheel to close idle connection */
	bool enableAutoClose = false;
	time::TimerInterval idleConnectionTimeout = 60000;
	time::TimerInterval halfCloseConnectionTimeout = 60000;
	// enable retry connect, max retry count
};

/**
 * @brief The global m_config for netpp application
 */
struct Config {
	/** @brief Event loop number */
	unsigned eventLoopNumber = 1;
// event loop wait interval
//	/** @brief Enable run user-defined handler in event loop */
//	bool enableRunHandlerInThread = false;
	// user-defined handler threads number
	/** @brief enable log */
	bool enableLog = false;
	struct {
		/** @brief Enable TickTimer */
		bool enable = true;
		/** @brief Wheel tick interval, in second */
		time::TimerInterval tickInterval = 1000;
		/** @brief The size of Wheel */
		time::TimerInterval maxLength = 600;
	} tickTimer;
	/** @brief Enable handle signal */
	bool enableHandleSignal = false;
	/** @brief Support dns resolve */
	bool enableDnsResolve = true;
	/** @brief The global event handler */
	netpp::Events eventHandler = {};
	ConnectionConfig connection = {};
// buffer node size
};
}

#endif
