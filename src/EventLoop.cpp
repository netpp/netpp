//
// Created by gaojian on 2020/7/5.
//

#include "EventLoop.h"
#include "internal/epoll/EpollEvent.h"
#include "internal/support/Log.h"
#include "internal/handlers/RunInLoopHandler.h"
#include "internal/time/TimeWheel.h"
#include "internal/epoll/Epoll.h"
#include <mutex>
#include <unordered_set>

namespace netpp {
/**
 * @brief Implement real event loop
 */
class EventLoopImpl {
public:
	EventLoopImpl();
	/**
	 * @brief Start the event loop
	 */
	void start();
	/**
	 * @brief Add one more event handler to loop
	 * @param handler The event handler, calls when interested event happened
	 */
	void add(const EventLoop::Handler &handler);
	/**
	 * @brief Remove a event handler from loop loop
	 * @param handler The event handler to be removed
	 */
	void remove(const EventLoop::Handler &handler);
	/**
	 * @brief Get poller
	 */
	internal::epoll::Epoll *getPoll() { return &m_poll; }

private:
	std::atomic_flag m_loopRunning;
	internal::epoll::Epoll m_poll;

	std::mutex m_handlersMutex;	// guard m_handlers
	std::unordered_set<EventLoop::Handler> m_handlers;	// epoll events handlers
};

EventLoop::EventLoop()
	: m_impl{std::make_unique<EventLoopImpl>()}
{
	// make runInLoopHandler needs m_poll, it must made after m_poll
	m_runInLoop = internal::handlers::RunInLoopHandler::makeRunInLoopHandler(this);
}

EventLoop::EventLoop(unsigned tickInterval, unsigned bucketCount)
	: EventLoop()
{
	m_kickIdleConnectionWheel = std::make_unique<internal::time::TimeWheel>(this, tickInterval, bucketCount);
}

EventLoop::~EventLoop() = default;

void EventLoop::run()
{
	m_impl->start();
}

void EventLoop::addEventHandlerToLoop(const Handler &handler)
{
	m_impl->add(handler);
}

void EventLoop::removeEventHandlerFromLoop(const Handler &handler)
{
	m_impl->remove(handler);
}

void EventLoop::runInLoop(std::function<void()> functor)
{
	m_runInLoop->addPendingFunction(std::move(functor));
}

internal::epoll::Epoll *EventLoop::getPoll()
{
	return m_impl->getPoll();
}

EventLoopImpl::EventLoopImpl()
	: m_loopRunning{false}
{}

void EventLoopImpl::start()
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
			std::vector<internal::epoll::EpollEvent *> activeChannels{4};
			using ChannelSize = std::vector<internal::epoll::EpollEvent *>::size_type;
			ChannelSize activeCount = m_poll.poll(activeChannels);
			for (ChannelSize i = 0; i < activeCount; ++i)
				activeChannels[i]->handleEvents();
//			for (auto &c : activeChannels)
//				c->handleEvents();
			activeChannels.clear();
		}
	}
	catch (...)
	{
		m_loopRunning.clear(std::memory_order_release);
		LOG_CRITICAL("Exception from event loop");
		throw;
	}
}

void EventLoopImpl::add(const EventLoop::Handler &handler)
{
	std::lock_guard lck(m_handlersMutex);
	m_handlers.insert(handler);
}

void EventLoopImpl::remove(const EventLoop::Handler &handler)
{
	std::lock_guard lck(m_handlersMutex);
	m_handlers.erase(handler);
}
}
