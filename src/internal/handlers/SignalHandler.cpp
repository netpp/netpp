#include "internal/handlers/SignalHandler.h"
#include "eventloop/EventLoop.h"
#include "internal/epoll/EpollEvent.h"
#include "signal/Signals.h"
#include "internal/stub/IO.h"
#include "signal/SignalWatcher.h"
#include "internal/support/Log.h"
extern "C" {
#include <sys/signalfd.h>
#include <csignal>
}

namespace netpp::internal::handlers {
SignalHandler::SignalHandler(eventloop::EventLoop *loop, Events eventsPrototype)
	: epoll::EventHandler(loop), m_events(std::move(eventsPrototype)), m_watchingSignals{new ::sigset_t}
{
	::sigemptyset(m_watchingSignals);
	m_signalFd = ::signalfd(-1, m_watchingSignals, SFD_NONBLOCK | SFD_CLOEXEC);
}

SignalHandler::~SignalHandler()
{
	::close(m_signalFd);
	::sigemptyset(m_watchingSignals);
	::pthread_sigmask(SIG_SETMASK, m_watchingSignals, nullptr);
	delete m_watchingSignals;
}

void SignalHandler::startWatchSignal(signal::Signals signal)
{
	::sigaddset(m_watchingSignals, toLinuxSignal(signal));
	::signalfd(m_signalFd, m_watchingSignals, SFD_NONBLOCK | SFD_CLOEXEC);
	::pthread_sigmask(SIG_SETMASK, m_watchingSignals, nullptr);
}

void SignalHandler::stopWatchSignal(signal::Signals signal)
{
	::sigdelset(m_watchingSignals, toLinuxSignal(signal));
	::signalfd(m_signalFd, m_watchingSignals, SFD_NONBLOCK | SFD_CLOEXEC);
	::pthread_sigmask(SIG_SETMASK, m_watchingSignals, nullptr);
}

void SignalHandler::handleIn()
{
	static constexpr int maxSignalRead = 20;
	static constexpr int readSize = sizeof(::signalfd_siginfo) * maxSignalRead;
	::signalfd_siginfo signals[maxSignalRead];
	::ssize_t readBytes = stub::read(m_signalFd, signals, readSize);
	if (readBytes != -1)
	{
		auto bytes = static_cast<unsigned>(readBytes);
		unsigned readNum = bytes / sizeof(::signalfd_siginfo);
		for (unsigned i = 0; i < readNum; ++i)
		{
			int signalNo = static_cast<int>(signals[i].ssi_signo);
			LOG_TRACE("signal {} occurred", signal::signalAsString(signalNo));
			// watching this signal
			if (signal::SignalWatcher::isWatching(signalNo))
				m_events.onSignal(signal::toNetppSignal(signalNo));// TODO: can pass more signal info to user
			else
				LOG_ERROR("not watching signal {}, but signal handler received it", signal::signalAsString(signalNo));
		}
	}
	else
	{
		LOG_WARN("read from signal fd failed");
	}
}

std::shared_ptr<SignalHandler> SignalHandler::makeSignalHandler(eventloop::EventLoop *loop, Events eventsPrototype)
{
	auto signalHandler = std::make_shared<SignalHandler>(loop, std::move(eventsPrototype));

	signalHandler->m_epollEvent = std::make_unique<epoll::EpollEvent>(
			loop->getPoll(), signalHandler,
			signalHandler->m_signalFd
	);
	loop->addEventHandlerToLoop(signalHandler);
	signalHandler->m_epollEvent->activeEvents(epoll::EpollEv::IN);
	LOG_TRACE("signal handler ready, fd {}", signal::SignalWatcher::signalFd);
	return signalHandler;
}
}
