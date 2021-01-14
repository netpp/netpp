#include "handlers/SignalHandler.h"
#include "signal/SignalPipe.h"
#include "EventLoop.h"
#include "epoll/EpollEvent.h"
extern "C" {
#include <unistd.h>
}

namespace netpp::handlers {
SignalHandler::SignalHandler()
	: EventHandler(-1)
{}

void SignalHandler::handleRead()
{
	constexpr int maxSignalRead = 100;
	int signals[maxSignalRead];
	int signalRead = 0;
	if (signal::SignalPipe::instance().isPipeOpened())
		signalRead = (::read(signal::SignalPipe::m_signalPipe[0], signals, sizeof(int) * maxSignalRead) / sizeof(int));
	for (int i = 0; i < signalRead; ++i)
		m_events->onSignal(signal::toNetppSignal(signals[i]));
}

void SignalHandler::makeSignalHandler(EventLoop *loop, std::unique_ptr<support::EventInterface> &&eventsPrototype)
{
	// construct SignalPipe
	signal::SignalPipe::instance();

	auto signalHandler = std::make_shared<SignalHandler>();
	signalHandler->_fd = signal::SignalPipe::instance().m_signalPipe[0];

	auto event = std::make_unique<epoll::EpollEvent>(loop->getPoll(), signalHandler);
	epoll::EpollEvent *eventPtr = event.get();

	signalHandler->m_events = std::move(eventsPrototype);
	signalHandler->m_epollEvent = std::move(event);

	eventPtr->setEnableRead(true);
	loop->addEventHandlerToLoop(signalHandler);
}
}
