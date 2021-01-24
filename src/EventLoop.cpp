//
// Created by gaojian on 2020/7/5.
//

#include "EventLoop.h"
#include "epoll/EpollEvent.h"
#include "Log.h"

namespace netpp {

thread_local EventLoop *EventLoop::_thisThreadLoop = nullptr;

EventLoop::EventLoop(unsigned tickInterval, unsigned bucketCount)
{
	m_kickIdleConnectionWheel = std::make_unique<time::TimeWheel>(this, tickInterval, bucketCount);
}

EventLoop::~EventLoop()
{
	_thisThreadLoop = nullptr;
}

[[noreturn]] void EventLoop::run()
{
	try
	{
		_thisThreadLoop = this;
		while (true)
		{
			std::vector<epoll::EpollEvent *> activeChannels = m_poll.poll();
			for (auto &c : activeChannels)
				c->handleEvents();
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
		SPDLOG_LOGGER_CRITICAL(logger, "Exeception from event loop");
		throw;
	}
}

void EventLoop::addEventHandlerToLoop(std::shared_ptr<epoll::EventHandler> handler)
{
	m_handlers.insert(handler);
}

void EventLoop::removeEventHandlerFromLoop(std::shared_ptr<epoll::EventHandler> handler)
{
	m_handlers.erase(handler);
}

void EventLoop::runInLoop(std::function<void()> functor)
{
	std::lock_guard lck(m_functorMutex);
	m_pendingFuns.emplace_back(functor);
}
}
