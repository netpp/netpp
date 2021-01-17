#include "signal/SignalWatcher.h"
#include "signal/SignalPipe.h"
#include "Log.h"
extern "C" {
#include <signal.h>
}

namespace netpp::signal {
/*class SignalSet {
public:
	SignalSet() { ::sigemptyset(&m_signalSet); }

	const sigset_t getSet() const
	{
		std::lock_guard lck(m_setMutex);
		return m_signalSet;
	}

	void add(int sig)
	{
		std::lock_guard lck(m_setMutex);
		::sigaddset(&m_signalSet, sig);
	}

	void del(int sig)
	{
		std::lock_guard lck(m_setMutex);
		::sigdelset(&m_signalSet, sig);
	}

private:
	static std::mutex m_setMutex;
	static ::sigset_t m_signalSet;
};
std::mutex SignalSet::m_setMutex;
::sigset_t SignalSet::m_signalSet;

// SignalWatcher
std::shared_ptr<SignalSet> SignalWatcher::m_watchingSignals = std::make_shared<SignalSet>();
*/

void SignalWatcher::watch(Signals signal)
{
	setSignalAction(signal, &signal::SignalPipe::handleSignal);
}

void SignalWatcher::restore(Signals signal)
{
	setSignalAction(signal, SIG_DFL);
}

void SignalWatcher::setSignalAction(Signals signal, void(*action)(int))
{
	struct ::sigaction act;
	act.sa_handler = action;
	act.sa_flags = SA_RESTART;
	::sigfillset(&act.sa_mask);
	// install action will always success
	::sigaction(toLinuxSignal(signal), &act, nullptr);
}
}
