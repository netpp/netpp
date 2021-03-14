//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EVENTLOOP_H
#define NETPP_EVENTLOOP_H

#include "epoll/Epoll.h"
#include <unordered_set>
#include <functional>
#include <mutex>
#include <memory>

namespace netpp {
namespace internal {
namespace epoll {
class EventHandler;
}
namespace time {
class TimeWheel;
}
namespace handlers {
class RunInLoopHandler;
}
}
class EventLoop {
public:
	using Handler = std::shared_ptr<internal::epoll::EventHandler>;

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
	~EventLoop();

	/**
	 * @brief Start run never ending event loop, however, the loop 
	 * will be terminated if any uncatched exception throwed, if 
	 * the loop is already running, return immediately
	 * 
	 */
	void run();

	/**
	 * @brief Add new event handler to event loop
	 * 
	 * @param handler The adding event handler
	 */
	void addEventHandlerToLoop(Handler handler);

	/**
	 * @brief Remove event handler from event loop.
	 * @note Remove handler from loop will not remove events from epoll,
	 * handlers need to clean it by themself.
	 * 
	 * @param handler The removing event handler
	 */
	void removeEventHandlerFromLoop(Handler handler);

	/// @brief Get poller object in this loop
	inline internal::epoll::Epoll *getPoll() { return &m_poll; }

	/**
	 * @brief Get time wheel in this loop
	 * @note It could be nullptr if not time wheel was created
	 * 
	 */
	internal::time::TimeWheel *getTimeWheel() { return m_kickIdleConnectionWheel.get(); }

	/// @brief Runs method in event loop
	void runInLoop(std::function<void()> functor);

private:
	std::atomic_flag m_loopRunning;
	internal::epoll::Epoll m_poll;

	std::mutex m_handlersMutex;	// guard m_handlers
	std::unordered_set<Handler> m_handlers;	// epoll events handlers

	std::shared_ptr<internal::handlers::RunInLoopHandler> m_runInLoop;
	std::unique_ptr<internal::time::TimeWheel> m_kickIdleConnectionWheel;
};
}

#endif //NETPP_EVENTLOOP_H
