#include "handlers/RunInLoopHandler.h"
#include "stub/IO.h"
#include "EventLoop.h"
#include "support/Log.h"
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
	m_epollEvent->disable();
	if (m_wakeUpFd != -1)
		stub::close(m_wakeUpFd);
}

void RunInLoopHandler::handleIn()
{
	::eventfd_t v;
	stub::eventfd_read(m_wakeUpFd, &v);
	LOG_INFO("Run in loop trigged, {} pending functors to run", v);
	std::vector<std::function<void()>> funs;
	{
		std::lock_guard lck(m_functorMutex);
		if (!m_pendingFuns.empty())
		{
			funs = m_pendingFuns;
			m_pendingFuns.clear();
		}
	}
	for (auto &f : funs)
		f();
}

void RunInLoopHandler::addPendingFunction(std::function<void()> functor)
{
	stub::eventfd_write(m_wakeUpFd, 1);
	std::lock_guard lck(m_functorMutex);
	m_pendingFuns.emplace_back(functor);
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
