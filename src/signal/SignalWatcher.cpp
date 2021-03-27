#include "signal/SignalWatcher.h"
#include "EventLoopDispatcher.h"
#include "internal/handlers/SignalHandler.h"
#include "internal/support/Log.h"
#include <mutex>
#include <cstring>
#include "Events.h"
#include "signal/Signals.h"
#include <memory>
#include <thread>
#include <condition_variable>
extern "C" {
#include <unistd.h>
#include <csignal>
#include <utility>
#include <sys/signalfd.h>
}

namespace netpp::signal {
// no lock required, signal fd will not change during runtime
int SignalWatcher::signalFd = -1;

::sigset_t *m_watchingSignals;
std::shared_ptr<internal::handlers::SignalHandler> m_signalHandler = nullptr;

std::thread m_unHandledSignalThread;
std::mutex m_watchSignalMutex;
/// @brief notify signal set changed
std::condition_variable m_waitWatchSignalChange;

SignalWatcher SignalWatcher::with(EventLoopDispatcher *dispatcher, Events eventsPrototype)
{
	if (m_signalHandler)
		m_signalHandler->stop();
	m_signalHandler = internal::handlers::SignalHandler::makeSignalHandler(dispatcher->dispatchEventLoop(), std::move(eventsPrototype));
	return SignalWatcher();
}

SignalWatcher SignalWatcher::watch(Signals signal)
{
	if (signalFd != -1)
	{
		std::unique_lock lck(m_watchSignalMutex);
		::sigaddset(m_watchingSignals, toLinuxSignal(signal));
		::signalfd(signalFd, m_watchingSignals, SFD_NONBLOCK | SFD_CLOEXEC);
		m_waitWatchSignalChange.notify_one();
	}
	return SignalWatcher();
}

SignalWatcher SignalWatcher::restore(Signals signal)
{
	if (signalFd != -1)
	{
		std::unique_lock lck(m_watchSignalMutex);
		::sigdelset(m_watchingSignals, toLinuxSignal(signal));
		::signalfd(signalFd, m_watchingSignals, SFD_NONBLOCK | SFD_CLOEXEC);
		m_waitWatchSignalChange.notify_one();
	}
	return SignalWatcher();
}

bool SignalWatcher::isWatching(Signals signal)
{
	if (signalFd != -1)
	{
		std::unique_lock lck(m_watchSignalMutex);
		return (::sigismember(m_watchingSignals, toLinuxSignal(signal)) == 1);
	}
	else
		return false;
}

bool SignalWatcher::isWatching(int signal)
{
	if (signalFd != -1)
	{
		std::unique_lock lck(m_watchSignalMutex);
		return (::sigismember(m_watchingSignals, signal) == 1);
	}
	else
		return false;
}

void SignalWatcher::enableWatchSignal()
{
	static std::once_flag setupWatchSignalFlag;
	std::call_once(setupWatchSignalFlag, []{
		// init watching signals
		{
			std::unique_lock lck(m_watchSignalMutex);
			m_watchingSignals = new ::sigset_t;
			::sigemptyset(m_watchingSignals);
		}
		// block all signals at very beginning, all thread will inherits this mask
		// no signal will send to threads created later
		::sigset_t blockThreadSignals;
		::sigfillset(&blockThreadSignals);
		::pthread_sigmask(SIG_SETMASK, &blockThreadSignals, nullptr);

		SignalWatcher::signalFd = ::signalfd(-1, m_watchingSignals, SFD_NONBLOCK | SFD_CLOEXEC);

		// enable netpp watch signal will create a new thread, who will block
		// watching signals only, other signals out side the block signal set
		// will be handled by default.
		if (SignalWatcher::signalFd != -1)
		{
			m_unHandledSignalThread = std::thread([]{
				while(true)
				{
					std::unique_lock lck(m_watchSignalMutex);
					::pthread_sigmask(SIG_SETMASK, m_watchingSignals, nullptr);
					m_waitWatchSignalChange.wait(lck);
					LOG_DEBUG("signal set changed, reset block");
				}
			});
			m_unHandledSignalThread.detach();
		}
	});
}
}
