#include "handlers/RunInLoopHandler.h"
#include "stub/IO.h"
#include "EventLoop.h"
extern "C" {
#include <fcntl.h>
}

namespace netpp::internal::handlers {
RunInLoopHandler::RunInLoopHandler(EventLoop *loop)
	: m_wakeUpFd{-1, -1}, m_waitingWakeUp{ATOMIC_FLAG_INIT}
{
	stub::pipe2(m_wakeUpFd, O_NONBLOCK);
	_loopThisHandlerLiveIn = loop;
}

RunInLoopHandler::~RunInLoopHandler()
{
	// no need to consider thread safety here, 
	// this handler will always live with event loop
	m_epollEvent->deactiveEvents();
	if (m_wakeUpFd[0] != -1)
		stub::close(m_wakeUpFd[0]);
	if (m_wakeUpFd[1] != -1)
		stub::close(m_wakeUpFd[1]);
}

void RunInLoopHandler::handleRead()
{
	m_waitingWakeUp.clear(std::memory_order_release);
	char c;
	stub::read(m_wakeUpFd[0], &c, sizeof(char));
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
	if (!m_waitingWakeUp.test_and_set(std::memory_order_consume))
	{
		char c;
		stub::write(m_wakeUpFd[1], &c, sizeof(char));
	}
	std::lock_guard lck(m_functorMutex);
	m_pendingFuns.emplace_back(functor);
}

std::shared_ptr<RunInLoopHandler> RunInLoopHandler::makeRunInLoopHandler(EventLoop *loop)
{
	auto handler = std::make_shared<RunInLoopHandler>(loop);
	handler->m_epollEvent = std::make_unique<epoll::EpollEvent>(loop->getPoll(), handler, handler->m_wakeUpFd[0]);
	loop->addEventHandlerToLoop(handler);
	// TODO: enable read only when add some pending functors
	handler->m_epollEvent->setEnableRead(true);
	return handler;
}
}
