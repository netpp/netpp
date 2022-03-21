#ifndef NETPP_SIGNAL_H
#define NETPP_SIGNAL_H

#include <string>

#define NETPP_SINGAL(XX) \
	XX(HUP)		\
	XX(INT)		\
	XX(QUIT)	\
	XX(ILL)		\
	XX(TRAP)	\
	XX(ABRT)	\
	XX(BUS)		\
	XX(FPE)		\
	XX(USR1)	\
	XX(SEGV)	\
	XX(USR2)	\
	XX(PIPE)	\
	XX(ALRM)	\
	XX(TERM)	\
	XX(STKFLT)	\
	XX(CHLD)	\
	XX(CONT)	\
	XX(TSTP)	\
	XX(TTIN)	\
	XX(TTOU)	\
	XX(URG)		\
	XX(XCPU)	\
	XX(XFSZ)	\
	XX(VTALRM)	\
	XX(PROF)	\
	XX(WINCH)	\
	XX(IO)		\
	XX(PWR)		\
	XX(SYS)

namespace netpp::signal {
/**
 * @brief enumerate Signals that can be handled(SIGKILL and SIGSTOP not included),
 * prefer use this than signal number
 * 
 */
enum class Signals {
#define SIGNAL_DEF(sig)	E_##sig,
NETPP_SINGAL(SIGNAL_DEF)
#undef SIGNAL_DEF
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

Signals toNetppSignal(int signal);

/// @brief Convert signal to string
std::string signalAsString(int signal);
std::string signalAsString(Signals signal);
}

#endif