//
// Created by gaojian on 2020/7/5.
//

#include "EventLoop.h"
#include "epoll/EpollEvent.h"
#include "support/Log.h"

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

void EventLoop::addEventHandlerToLoop(std::shared_ptr<internal::epoll::EventHandler> handler)
{
	m_handlers.insert(handler);
}

void EventLoop::removeEventHandlerFromLoop(std::shared_ptr<internal::epoll::EventHandler> handler)
{
	// TODO: remove fd from epoll before erase
	// FIXME: make this thread safe
	m_handlers.erase(handler);
}

void EventLoop::runInLoop(std::function<void()> functor)
{
	m_runInLoop->addPendingFunction(functor);
}
}
