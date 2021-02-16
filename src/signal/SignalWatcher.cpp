#include "signal/SignalWatcher.h"
#include "EventLoopDispatcher.h"
#include "handlers/SignalHandler.h"
#include <mutex>
extern "C" {
#include <signal.h>
#include <sys/signalfd.h>
}

namespace netpp::signal {
// no lock required, signal fd will not change during runtime
int SignalWatcher::signalFd = -1;
volatile std::atomic_uint64_t SignalWatcher::m_watchingSignals = 0;
std::shared_ptr<handlers::SignalHandler> SignalWatcher::m_signalHandler = nullptr;

static constexpr uint64_t uint64One = 1;

SignalWatcher SignalWatcher::with(EventLoopDispatcher *dispatcher, Events eventsPrototype)
{
	if (m_signalHandler)
		m_signalHandler->handleClose();
	m_signalHandler = handlers::SignalHandler::makeSignalHandler(dispatcher->dispatchEventLoop(), eventsPrototype);
	return SignalWatcher();
}

SignalWatcher SignalWatcher::watch(Signals signal)
{
	if (signalFd != -1)
		m_watchingSignals.fetch_or((uint64One << toLinuxSignal(signal)), std::memory_order_relaxed);
	return SignalWatcher();
}

SignalWatcher SignalWatcher::restore(Signals signal)
{
	if (signalFd != -1)
		m_watchingSignals.fetch_and(~(uint64One << toLinuxSignal(signal)), std::memory_order_relaxed);
	return SignalWatcher();
}

bool SignalWatcher::isWatching(Signals signal)
{
	if (signalFd != -1)
		return (m_watchingSignals.load(std::memory_order_relaxed) & (uint64One << toLinuxSignal(signal)));
	else
		return false;
}

bool SignalWatcher::isWatching(uint32_t signal)
{
	if (signalFd != -1)
		return (m_watchingSignals.load(std::memory_order_relaxed) & (uint64One << signal));
	else
		return false;
}

void SignalWatcher::enableWatchSignal()
{
	static std::once_flag setupWatchSignalFlag;
	std::call_once(setupWatchSignalFlag, []{
		// block all signals at very beginning, all thread will inherits this mask
		::sigset_t blockThreadSignals;
		::sigfillset(&blockThreadSignals);
		::pthread_sigmask(SIG_SETMASK, &blockThreadSignals, nullptr);
		SignalWatcher::signalFd = ::signalfd(-1, &blockThreadSignals, SFD_NONBLOCK);
	});
}
}
