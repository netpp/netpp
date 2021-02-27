//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EVENTLOOP_H
#define NETPP_EVENTLOOP_H

#include "epoll/Epoll.h"
#include "time/TimeWheel.h"
#include "handlers/RunInLoopHandler.h"
#include <unordered_set>
#include <functional>
#include <mutex>
#include <memory>

namespace netpp {
namespace internal::epoll {
class EventHandler;
}
class EventLoop {
public:
	/**
	 * @brief Default EventLoop, will not create timewheel
	 * @throw ResourceLimitException
	 * 1. file descriptors limit
	 * 2. no more memory
	 */
	EventLoop();

	/**
	 * @brief Construct an EventLoop with timewheel
	 * 
	 * @param tickInterval	Wheel rotate interval, by milliseconds
	 * @param bucketCount	Contains n buckets
	 */
	EventLoop(unsigned tickInterval, unsigned bucketCount);
	~EventLoop() = default;

	/**
	 * @brief start run never ending event loop, however, the loop 
	 * will be terminated if any uncatched exception throwed, if 
	 * the loop is already running, return immediately
	 * 
	 */
	void run();

	/**
	 * @brief add new event handler to event loop
	 * 
	 * @param handler the adding event handler
	 */
	void addEventHandlerToLoop(std::shared_ptr<internal::epoll::EventHandler> handler);

	/**
	 * @brief remove event handler from event loop
	 * 
	 * @param handler the removing event handler
	 */
	void removeEventHandlerFromLoop(std::shared_ptr<internal::epoll::EventHandler> handler);

	/// @brief get poller object in this loop
	inline internal::epoll::Epoll *getPoll() { return &m_poll; }

	/**
	 * @brief get time wheel in this loop
	 * @note it could be nullptr if not time wheel was created
	 * 
	 */
	internal::time::TimeWheel *getTimeWheel() { return m_kickIdleConnectionWheel.get(); }

	/// @brief runs method in event loop
	void runInLoop(std::function<void()> functor);

private:
	std::atomic_flag m_loopRunning;
	internal::epoll::Epoll m_poll;
	std::unordered_set<std::shared_ptr<internal::epoll::EventHandler>> m_handlers;	// epoll events handlers

	std::shared_ptr<internal::handlers::RunInLoopHandler> m_runInLoop;
	std::unique_ptr<internal::time::TimeWheel> m_kickIdleConnectionWheel;
};
}

#endif //NETPP_EVENTLOOP_H
