//
// Created by gaojian on 2020/7/5.
//

#include "eventloop/EventLoop.h"
#include "epoll/EpollEventHandler.h"
#include "support/Log.h"
#include "epoll/Epoll.h"
#include "epoll/handlers/RunInLoopHandler.h"
#include "eventloop/EventLoopData.h"

namespace {
thread_local netpp::EventLoop *thisEventLoopOnThread = nullptr;
}

namespace netpp {
EventLoop::EventLoop()
	: m_loopRunning{false}, m_poll{std::make_unique<Epoll>()}, m_loopKeepRunning{false}
{
}

EventLoop::~EventLoop() = default;

void EventLoop::run()
{
	if (m_loopRunning.test_and_set(std::memory_order_consume))
	{
		LOG_INFO("Event loop is running in this thread");
		return;
	}
	::thisEventLoopOnThread = this;
	m_loopKeepRunning = true;
	while (m_loopKeepRunning)
	{
		m_poll->poll();
	}
	m_loopRunning.clear();
}

void EventLoop::quit()
{
	runInLoop([this]{
		m_loopKeepRunning = false;
	});
}

void EventLoop::addEventHandlerToLoop(const Handler &handler)
{
	std::lock_guard lck(m_handlersMutex);
	m_handlers.insert(handler);
}

void EventLoop::removeEventHandlerFromLoop(const Handler &handler)
{
	std::lock_guard lck(m_handlersMutex);
	m_handlers.erase(handler);
}

Epoll *EventLoop::getPoll()
{
	return m_poll.get();
}

EventLoop *EventLoop::thisLoop()
{
	return ::thisEventLoopOnThread;
}

void EventLoop::runInLoop(std::function<void()> task)
{
	if (EventLoop::thisLoop() == this)
		task();
	else
	{
		if (m_loopData)
		{
			if (m_loopData->runInLoopHandler)
				m_loopData->runInLoopHandler->addPendingFunction(std::move(task));
		}
	}
}
}
