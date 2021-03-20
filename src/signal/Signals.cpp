#include "signal/Signals.h"
extern "C" {
#include <signal.h>
#include <string.h>
}

namespace netpp::signal {
int toLinuxSignal(Signals signal)
{
#ifdef SIGNAL_DEF
#undef SIGNAL_DEF
#endif
#ifdef LAST_SIGNAL_DEF
#undef LAST_SIGNAL_DEF
#endif
#define SIGNAL_DEF(type)		case Signals::E_##type: sig = SIG##type; break;
#define LAST_SIGNAL_DEF(type)	SIGNAL_DEF(type)
	int sig = -1;
	switch (signal)
	{
#include "signal/Signal.def"
	}
	return sig;
}

Signals toNetppSignal(int signal)
{
#ifdef SIGNAL_DEF
#undef SIGNAL_DEF
#endif
#ifdef LAST_SIGNAL_DEF
#undef LAST_SIGNAL_DEF
#endif
#define SIGNAL_DEF(type)		case SIG##type: sig = Signals::E_##type; break;
#define LAST_SIGNAL_DEF(type)	SIGNAL_DEF(type)
	Signals sig;
	switch (signal)
	{
#include "signal/Signal.def"
	}
	return sig;
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
