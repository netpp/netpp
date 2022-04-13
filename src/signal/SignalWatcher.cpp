#include "signal/SignalWatcher.h"
#include "internal/handlers/SignalHandler.h"
#include "internal/support/Log.h"
#include <mutex>
#include <cstring>
#include "Events.h"
#include "signal/Signals.h"
#include "eventloop/EventLoopManager.h"
#include "Application.h"
#include "eventloop/EventLoop.h"
extern "C" {
#include <unistd.h>
#include <csignal>
#include <sys/signalfd.h>
}

namespace netpp::signal {
std::mutex m_signalMutex;
::sigset_t *m_watchingSignals;

void SignalWatcher::watch(Signals signal)
{
	std::lock_guard lck(m_signalMutex);
	::sigaddset(m_watchingSignals, toLinuxSignal(signal));
	auto manager = Application::loopManager();
	auto mainLoop = manager->mainLoop();
	auto loopData = dynamic_cast<eventloop::MainEventLoopData *>(manager->getLoopData(mainLoop));
	auto handler = loopData->signalHandler->weak_from_this();
	mainLoop->runInLoop([signal, handler]{
		auto signalHandler = handler.lock();
		if (signalHandler)
			signalHandler->startWatchSignal(signal);
	});
}

void SignalWatcher::restore(Signals signal)
{
	std::lock_guard lck(m_signalMutex);
	::sigdelset(m_watchingSignals, toLinuxSignal(signal));
	auto manager = Application::loopManager();
	auto mainLoop = manager->mainLoop();
	auto loopData = dynamic_cast<eventloop::MainEventLoopData *>(manager->getLoopData(mainLoop));
	auto handler = loopData->signalHandler->weak_from_this();
	mainLoop->runInLoop([signal, handler]{
		auto signalHandler = handler.lock();
		if (signalHandler)
			signalHandler->stopWatchSignal(signal);
	});
}

bool SignalWatcher::isWatching(Signals signal)
{
	std::lock_guard lck(m_signalMutex);
	return (::sigismember(m_watchingSignals, toLinuxSignal(signal)) == 1);
}

bool SignalWatcher::isWatching(int signal)
{
	std::lock_guard lck(m_signalMutex);
	return (::sigismember(m_watchingSignals, signal) == 1);
}

void SignalWatcher::enableWatchSignal()
{
	static std::once_flag setupWatchSignalFlag;
	std::call_once(setupWatchSignalFlag, []{
		// init watching signals with empty set
		{
			std::lock_guard lck(m_signalMutex);
			m_watchingSignals = new ::sigset_t;
			::sigemptyset(m_watchingSignals);
		}
		// block all signals at very beginning, all thread will inherit this mask,
		// no signal will send to thread create later
		::sigset_t blockThreadSignals;
		::sigfillset(&blockThreadSignals);
		::pthread_sigmask(SIG_SETMASK, &blockThreadSignals, nullptr);
	});
}
}
