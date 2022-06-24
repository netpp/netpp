#include "epoll/handlers/RunInLoopHandler.h"
#include "internal/stub/IO.h"
#include "eventloop/EventLoop.h"
#include "support/Log.h"
#include "error/Exception.h"
extern "C" {
#include <fcntl.h>
}

namespace netpp {
RunInLoopHandler::RunInLoopHandler(EventLoop *loop)
		: EpollEventHandler(loop)
{
	m_wakeUpFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (m_wakeUpFd == -1)
	{
		switch (errno)
		{
			case EINVAL:
				break;
			case EMFILE:
			case ENFILE:
			case ENODEV:
			case ENOMEM:
				throw ResourceLimitException(errno);
		}
	}
	activeEvents(EpollEv::IN);
}

RunInLoopHandler::~RunInLoopHandler()
{
	// no need to consider thread safety here, 
	// this handler will always live with event loop
	if (m_wakeUpFd != -1)
		stub::close(m_wakeUpFd);
}

void RunInLoopHandler::handleIn()
{
	::eventfd_t v;
	::eventfd_read(m_wakeUpFd, &v);
	LOG_DEBUG("Run in loop triggered, {} pending functors to run", v);

	std::vector<std::function<void()>> functors;
	{
		std::lock_guard lck(m_functorMutex);
		if (!m_pendingFunctors.empty())
		{
			functors = m_pendingFunctors;
			m_pendingFunctors.clear();
		}
	}
	for (auto &f: functors)
	{
		f();
	}
}

void RunInLoopHandler::addPendingFunction(std::function<void()> functor)
{
	::eventfd_write(m_wakeUpFd, 1);
	std::lock_guard lck(m_functorMutex);
	m_pendingFunctors.emplace_back(std::move(functor));
}
}
