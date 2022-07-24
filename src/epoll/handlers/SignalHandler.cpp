#include "epoll/handlers/SignalHandler.h"
#include "eventloop/EventLoop.h"
#include "signal/Signals.h"
#include "internal/stub/IO.h"
#include "support/Log.h"
#include <utility>
extern "C" {
#include <sys/signalfd.h>
#include <csignal>
}

namespace netpp {
SignalHandler::SignalHandler(EventLoop *loop, const std::initializer_list<netpp::Signals> &interestedSignals)
	: EpollEventHandler(loop)
{
	::sigset_t blockThreadSignals;
	::sigemptyset(&blockThreadSignals);
	// block signals at very beginning, all thread will inherit this mask,
	// no signal will send to thread create later
	for (auto &s : interestedSignals)
		::sigaddset(&blockThreadSignals, toLinuxSignal(s));
	::pthread_sigmask(SIG_SETMASK, &blockThreadSignals, nullptr);
	m_signalFd = ::signalfd(-1, &blockThreadSignals, SFD_NONBLOCK | SFD_CLOEXEC);

	activeEvents(EpollEv::IN);
}

SignalHandler::~SignalHandler()
{
	::close(m_signalFd);
	::sigset_t blockThreadSignals;
	::sigemptyset(&blockThreadSignals);
	::pthread_sigmask(SIG_SETMASK, &blockThreadSignals, nullptr);
}

void SignalHandler::setSignalCallback(SignalCallBack cb)
{
	std::lock_guard lck(m_callBackMutex);
	m_callback = std::move(cb);
}

void SignalHandler::handleIn()
{
	static constexpr int maxSignalRead = 20;
	static constexpr int readSize = sizeof(::signalfd_siginfo) * maxSignalRead;
	::signalfd_siginfo signals[maxSignalRead];
	::ssize_t readBytes = ::read(m_signalFd, signals, readSize);
	if (readBytes != -1)
	{
		auto bytes = static_cast<unsigned>(readBytes);
		unsigned readNum = bytes / sizeof(::signalfd_siginfo);
		for (unsigned i = 0; i < readNum; ++i)
		{
			int signalNo = static_cast<int>(signals[i].ssi_signo);
			LOG_TRACE("signal {} occurred", signal::signalAsString(signalNo));
			SignalCallBack cb;
			{
				std::lock_guard lck(m_callBackMutex);
				cb = m_callback;
			}
			if (cb)
				cb(toNetppSignal(signalNo));// TODO: can pass more signal info to user
		}
	}
	else
	{
		LOG_WARN("read from signal fd failed");
	}
}
}
