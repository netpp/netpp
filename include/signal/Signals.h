#ifndef NETPP_SIGNAL_H
#define NETPP_SIGNAL_H

#include <string>

namespace netpp::signal {
/**
 * @brief enumerate Signals that can be handled(SIGKILL and SIGSTOP not included),
 * prefer use this than signal number
 * 
 */
enum class Signals {
#ifdef SIGNAL_DEF
#undef SIGNAL_DEF
#endif
#ifdef LAST_SIGNAL_DEF
#undef LAST_SIGNAL_DEF
#endif
#define SIGNAL_DEF(sig)	E_##sig,
#define LAST_SIGNAL_DEF(sig) E_##sig
#include "Signal.def"
};

/**
 * @brief Convert Signals to signal number
 * 
 * @param signal	enumerated signal
 * @return int		signal number
 */
int toLinuxSignal(Signals signal);

/**
 * @brief Convert signal number to Signals
 * 
 * @param signal	signal number
 * @return Signals	enumerated signal
 */

Signals toNetppSignal(uint32_t signal);

/// @brief Convert signal to string
std::string signalAsString(uint32_t signal);
std::string signalAsString(Signals signal);
}

#endif