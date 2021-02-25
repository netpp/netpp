//
// Created by gaojian on 2020/7/5.
//

#include "EventLoop.h"
#include "epoll/EpollEvent.h"
#include "support/Log.h"

namespace netpp {

EventLoop::EventLoop(unsigned tickInterval, unsigned bucketCount)
{
	m_kickIdleConnectionWheel = std::make_unique<internal::time::TimeWheel>(this, tickInterval, bucketCount);
}

[[noreturn]] void EventLoop::run()
{
	try
	{
		// FIXME: can only run in single thread
		while (true)
		{
			std::vector<internal::epoll::EpollEvent *> activeChannels = m_poll.poll();
			for (auto &c : activeChannels)
				c->handleEvents();
			activeChannels.clear();
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
	}
	catch (...)
	{
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
	// TODO: remove fd from epoll
	// FIXME: make this thread safe
	m_handlers.erase(handler);
}

void EventLoop::runInLoop(std::function<void()> functor)
{
	// TODO: wake up epoll_wait via some fd, not waiting for epoll_wait timeout
	std::lock_guard lck(m_functorMutex);
	m_pendingFuns.emplace_back(functor);
}
}
