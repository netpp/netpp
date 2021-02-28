//
// Created by gaojian on 2020/7/5.
//

#include "EventLoop.h"
#include "epoll/EpollEvent.h"
#include "support/Log.h"
#include "handlers/RunInLoopHandler.h"

namespace netpp {
EventLoop::EventLoop()
	: m_loopRunning{ATOMIC_FLAG_INIT}
{
	m_runInLoop = internal::handlers::RunInLoopHandler::makeRunInLoopHandler(this);
}

EventLoop::EventLoop(unsigned tickInterval, unsigned bucketCount)
	: EventLoop()
{
	m_kickIdleConnectionWheel = std::make_unique<internal::time::TimeWheel>(this, tickInterval, bucketCount);
}

void EventLoop::run()
{
	try
	{
		if (m_loopRunning.test_and_set(std::memory_order_consume))
		{
			LOG_INFO("Event loop is running in this thread");
			return;
		}
		while (true)
		{
			std::vector<internal::epoll::EpollEvent *> activeChannels = m_poll.poll();
			for (auto &c : activeChannels)
				c->handleEvents();
			activeChannels.clear();
		}
	}
	catch (...)
	{
		m_loopRunning.clear(std::memory_order_release);
		LOG_CRITICAL("Exeception from event loop");
		throw;
	}
}

void EventLoop::addEventHandlerToLoop(Handler handler)
{
	std::lock_guard lck(m_handlersMutex);
	m_handlers.insert(handler);
}

void EventLoop::removeEventHandlerFromLoop(Handler handler)
{
	std::lock_guard lck(m_handlersMutex);
	m_handlers.erase(handler);
}

void EventLoop::runInLoop(std::function<void()> functor)
{
	m_runInLoop->addPendingFunction(functor);
}
}
