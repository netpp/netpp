#include "signal/SignalFd.h"
#include "Log.h"
#include "stub/IO.h"
extern "C" {
#include <signal.h>
#include <sys/signalfd.h>
}

namespace netpp::signal {
SignalFd SignalFd::instance;
int SignalFd::signalFd = -1;
volatile uint64_t SignalFd::m_watchingSignals = 0;

SignalFd::SignalFd() noexcept
{
	// block all signals at very beginning, all thread will inherits this mask
	::sigset_t blockThreadSignals;
	::sigfillset(&blockThreadSignals);
	::pthread_sigmask(SIG_SETMASK, &blockThreadSignals, nullptr);
	signalFd = ::signalfd(-1, &blockThreadSignals, SFD_NONBLOCK);
}

void SignalFd::add(int sig)
{
	m_watchingSignals |= (1u << sig);
}

void SignalFd::del(int sig)
{
	m_watchingSignals &= ~(1u << sig);
}

bool SignalFd::watching(int sig)
{
	return (m_watchingSignals & (1u << sig));
}
}
