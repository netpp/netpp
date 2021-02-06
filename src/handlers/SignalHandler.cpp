#include "handlers/SignalHandler.h"
#include "EventLoop.h"
#include "epoll/EpollEvent.h"
#include "signal/Signals.h"
#include "stub/IO.h"
#include "signal/SignalWatcher.h"
#include "error/Exception.h"
#include "Log.h"
extern "C" {
#include <sys/signalfd.h>
}

namespace netpp::handlers {
void SignalHandler::handleRead()
{
	static constexpr int maxSignalRead = 20;
	::signalfd_siginfo signals[maxSignalRead];
	int readBytes = 0;
	readBytes = stub::read(
		signal::SignalWatcher::signalFd,
		signals,
		sizeof(::signalfd_siginfo) * maxSignalRead
	);
	int readNum = readBytes / sizeof(::signalfd_siginfo);
	for (int i = 0; i < readNum; ++i)
	{
		SPDLOG_LOGGER_TRACE(logger, "signal {} occurred", signal::signalAsString(signals[i].ssi_signo));
		// watching this signal
		if (signal::SignalWatcher::isWatching(signals[i].ssi_signo))
			m_events->onSignal(signal::toNetppSignal(signals[i].ssi_signo));// TODO: can pass more signal info to user
		else if (!signal::ignoreByDefault(signals[i].ssi_signo))
			throw error::UnhandledSignal(signals[i]);
	}
}

void SignalHandler::makeSignalHandler(EventLoop *loop, std::unique_ptr<support::EventInterface> &&eventsPrototype)
{
	auto signalHandler = std::make_shared<SignalHandler>();
	auto event = std::make_unique<epoll::EpollEvent>(
		loop->getPoll(), signalHandler,
		signal::SignalWatcher::signalFd
	);
	epoll::EpollEvent *eventPtr = event.get();

	signalHandler->m_events = std::move(eventsPrototype);
	signalHandler->m_epollEvent = std::move(event);

	eventPtr->setEnableRead(true);
	loop->addEventHandlerToLoop(signalHandler);
	SPDLOG_LOGGER_TRACE(logger, "signal handler ready, fd {}", signal::SignalWatcher::signalFd);
}
}
