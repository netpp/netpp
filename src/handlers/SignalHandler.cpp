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
void SignalHandler::handleIn()
{
	static constexpr int maxSignalRead = 20;
	::signalfd_siginfo signals[maxSignalRead];
	::ssize_t readBytes = stub::read(
		signal::SignalWatcher::signalFd,
		signals,
		sizeof(::signalfd_siginfo) * maxSignalRead
	);
	if (readBytes != -1)
	{
		unsigned bytes = static_cast<unsigned>(readBytes);
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

void SignalHandler::stop()
{
	auto externLife = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([externLife](){
		externLife->m_epollEvent->disable();
		externLife->_loopThisHandlerLiveIn->removeEventHandlerFromLoop(externLife);
	});
}

std::shared_ptr<SignalHandler> SignalHandler::makeSignalHandler(EventLoop *loop, Events eventsPrototype)
{
	auto signalHandler = std::make_shared<SignalHandler>();
	signalHandler->m_events = eventsPrototype;
	signalHandler->m_epollEvent = std::make_unique<epoll::EpollEvent>(
		loop->getPoll(), signalHandler,
		signal::SignalWatcher::signalFd
	);
	signalHandler->_loopThisHandlerLiveIn = loop;

	loop->runInLoop([signalHandler]{
		signalHandler->_loopThisHandlerLiveIn->addEventHandlerToLoop(signalHandler);
		signalHandler->m_epollEvent->active(epoll::Event::IN);
		LOG_TRACE("signal handler ready, fd {}", signal::SignalWatcher::signalFd);
	});
	return signalHandler;
}
}
