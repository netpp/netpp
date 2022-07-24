#include "signal/Signals.h"
#include <mutex>
extern "C" {
#include <csignal>
#include <cstring>
}

namespace netpp {
int toLinuxSignal(Signals signal)
{
#define SIGNAL_DEF(type)		case Signals::E_##type: sig = SIG##type; break;
	int sig = -1;
	switch (signal)
	{
		NETPP_SINGAL(SIGNAL_DEF)
	}
	return sig;
#undef SIGNAL_DEF
}

Signals toNetppSignal(int signal)
{
#define SIGNAL_DEF(type)		case SIG##type: sig = Signals::E_##type; break;
	Signals sig;
	switch (signal)
	{
		NETPP_SINGAL(SIGNAL_DEF)
		default:
			sig = Signals::E_HUP;
			break;
	}
	return sig;
#undef SIGNAL_DEF
}

std::string signalAsString(int signal)
{
	if (signal < 0 || signal > SIGRTMAX)
		return "";
#if __GLIBC_PREREQ(2, 32)
	return ::sigdescr_np(signal);
#else
	static std::mutex mutex;
	std::lock_guard lck(mutex);
	return ::strsignal(signal);
#endif
}

std::string signalAsString(Signals signal)
{
	return signalAsString(toLinuxSignal(signal));
}
}
