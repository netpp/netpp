#include "internal/handlers/SignalHandler.h"
#include "eventloop/EventLoop.h"
#include "internal/epoll/EpollEvent.h"
#include "signal/Signals.h"
#include "internal/stub/IO.h"
#include "internal/support/Log.h"
extern "C" {
#include <sys/signalfd.h>
#include <csignal>
}

namespace netpp::internal::handlers {
SignalHandler::SignalHandler(eventloop::EventLoop *loop, Events eventsPrototype, const std::vector<netpp::signal::Signals> &interestedSignals)
	: epoll::EventHandler(loop), m_events(std::move(eventsPrototype))
{
	static std::once_flag setupWatchSignalFlag;
	::sigset_t blockThreadSignals;
	::sigemptyset(&blockThreadSignals);
	// block signals at very beginning, all thread will inherit this mask,
	// no signal will send to thread create later
	for (auto &s : interestedSignals)
		::sigaddset(&blockThreadSignals, toLinuxSignal(s));
	::pthread_sigmask(SIG_SETMASK, &blockThreadSignals, nullptr);
	m_signalFd = ::signalfd(-1, &blockThreadSignals, SFD_NONBLOCK | SFD_CLOEXEC);
}

SignalHandler::~SignalHandler()
{
	::close(m_signalFd);
	::sigset_t blockThreadSignals;
	::sigemptyset(&blockThreadSignals);
	::pthread_sigmask(SIG_SETMASK, &blockThreadSignals, nullptr);
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
			m_events.onSignal(signal::toNetppSignal(signalNo));// TODO: can pass more signal info to user
		}
	}
	else
	{
		LOG_WARN("read from signal fd failed");
	}
}

std::shared_ptr<SignalHandler> SignalHandler::makeSignalHandler(eventloop::EventLoop *loop, Events eventsPrototype, const std::vector<netpp::signal::Signals> &interestedSignals)
{
	auto signalHandler = std::make_shared<SignalHandler>(loop, std::move(eventsPrototype), interestedSignals);

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
