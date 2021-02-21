#include "handlers/SignalHandler.h"
#include "EventLoop.h"
#include "epoll/EpollEvent.h"
#include "signal/Signals.h"
#include "stub/IO.h"
#include "signal/SignalWatcher.h"
#include "error/Exception.h"
#include "support/Log.h"
#include "EventLoop.h"
extern "C" {
#include <sys/signalfd.h>
}

namespace netpp::internal::handlers {
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
		LOG_TRACE("signal {} occurred", signal::signalAsString(signals[i].ssi_signo));
		// watching this signal
		if (signal::SignalWatcher::isWatching(signals[i].ssi_signo))
			m_events.onSignal(signal::toNetppSignal(signals[i].ssi_signo));// TODO: can pass more signal info to user
		else
			LOG_ERROR("not watching signal {}, but signal handler received it", signal::signalAsString(signals[i].ssi_signo));
	}
}

void SignalHandler::handleClose()
{
	m_epollEvent->deactiveEvents();
	volatile auto externLife = shared_from_this();
	_loopThisHandlerLiveIn->removeEventHandlerFromLoop(shared_from_this());
}

std::shared_ptr<SignalHandler> SignalHandler::makeSignalHandler(EventLoop *loop, Events eventsPrototype)
{
	auto signalHandler = std::make_shared<SignalHandler>();
	auto event = std::make_unique<epoll::EpollEvent>(
		loop->getPoll(), signalHandler,
		signal::SignalWatcher::signalFd
	);
	epoll::EpollEvent *eventPtr = event.get();

	signalHandler->m_events = eventsPrototype;
	signalHandler->m_epollEvent = std::move(event);
	signalHandler->_loopThisHandlerLiveIn = loop;

	loop->addEventHandlerToLoop(signalHandler);
	eventPtr->setEnableRead(true);
	LOG_TRACE("signal handler ready, fd {}", signal::SignalWatcher::signalFd);
	return signalHandler;
}
}
