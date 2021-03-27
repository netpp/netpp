#include "internal/handlers/RunInLoopHandler.h"
#include "internal/stub/IO.h"
#include "EventLoop.h"
#include "internal/support/Log.h"
extern "C" {
#include <fcntl.h>
}

namespace netpp::internal::handlers {
RunInLoopHandler::RunInLoopHandler(EventLoop *loop)
{
	m_wakeUpFd = stub::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	_loopThisHandlerLiveIn = loop;
}

RunInLoopHandler::~RunInLoopHandler()
{
	// no need to consider thread safety here, 
	// this handler will always live with event loop
	if (m_epollEvent)
		m_epollEvent->disable();
	if (m_wakeUpFd != -1)
		stub::close(m_wakeUpFd);
}

void RunInLoopHandler::handleIn()
{
	::eventfd_t v;
	stub::eventfd_read(m_wakeUpFd, &v);
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
	for (auto &f : functors)
		f();
}

void RunInLoopHandler::addPendingFunction(std::function<void()> functor)
{
	stub::eventfd_write(m_wakeUpFd, 1);
	std::lock_guard lck(m_functorMutex);
	m_pendingFunctors.emplace_back(std::move(functor));
}

std::shared_ptr<RunInLoopHandler> RunInLoopHandler::makeRunInLoopHandler(EventLoop *loop)
{
	auto handler = std::make_shared<RunInLoopHandler>(loop);
	handler->m_epollEvent = std::make_unique<epoll::EpollEvent>(loop->getPoll(), handler, handler->m_wakeUpFd);
	loop->addEventHandlerToLoop(handler);
	handler->m_epollEvent->active(epoll::Event::IN);
	return handler;
}
}
