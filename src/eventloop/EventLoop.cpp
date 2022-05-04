//
// Created by gaojian on 2020/7/5.
//

#include "eventloop/EventLoop.h"
#include "internal/epoll/EpollEvent.h"
#include "internal/support/Log.h"
#include "internal/epoll/Epoll.h"
#include "internal/handlers/RunInLoopHandler.h"
#include "eventloop/EventLoopData.h"

namespace {
thread_local netpp::eventloop::EventLoop *thisEventLoopOnThread = nullptr;
}

namespace netpp::eventloop {
EventLoop::EventLoop()
	: m_loopRunning{false}, m_poll{std::make_unique<internal::epoll::Epoll>()}
{
}

EventLoop::~EventLoop() = default;

void EventLoop::run()
{
	try
	{
		if (m_loopRunning.test_and_set(std::memory_order_consume))
		{
			LOG_INFO("Event loop is running in this thread");
			return;
		}
		::thisEventLoopOnThread = this;
		std::vector<internal::epoll::EpollEvent *> activeChannels{4};
		while (true)
		{
			using ChannelSize = std::vector<internal::epoll::EpollEvent *>::size_type;
			ChannelSize activeCount = m_poll->poll(activeChannels);
			for (ChannelSize i = 0; i < activeCount; ++i)
				activeChannels[i]->handleEvents();
//			for (auto &c : activeChannels)
//				c->handleEvents();
			activeChannels.clear();
		}
	}
	catch (...)
	{
		::thisEventLoopOnThread = nullptr;
		m_loopRunning.clear(std::memory_order_release);
		LOG_CRITICAL("Exception from event loop");
		throw;
	}
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

internal::epoll::Epoll *EventLoop::getPoll()
{
	return m_poll.get();
}

EventLoop *EventLoop::thisLoop()
{
	return ::thisEventLoopOnThread;
}

void EventLoop::runInLoop(std::function<void()> task)
{
	if (m_loopData)
	{
		if (m_loopData->runInLoopHandler)
			m_loopData->runInLoopHandler->addPendingFunction(std::move(task));
	}
}
}
