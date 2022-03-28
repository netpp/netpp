//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EVENTLOOP_H
#define NETPP_EVENTLOOP_H

#include <functional>
#include <memory>

namespace netpp {
namespace internal {
namespace epoll {
class EventHandler;
class Epoll;
}
namespace time {
class TimeWheel;
}
namespace handlers {
class RunInLoopHandler;
}
}
class EventLoopImpl;
/**
 * @brief The event loop wait/dispatch/handle events.
 *
 * Event loop runs only in one thread, and a thread can only run one event loop
 */
class EventLoop {
public:
	/**
	 * @brief Handler type
	 */
	using Handler = std::shared_ptr<internal::epoll::EventHandler>;
	/**
	 * @brief Default EventLoop, will not create time wheel
	 * @throw ResourceLimitException
	 * 1. file descriptors limit
	 * 2. no more memory
	 */
	EventLoop();

	/**
	 * @brief Construct an EventLoop with time wheel
	 * 
	 * @param tickInterval	Wheel rotate interval, by milliseconds
	 * @param bucketCount	Contains n buckets
	 */
	EventLoop(unsigned tickInterval, unsigned bucketCount);
	~EventLoop();

	/**
	 * @brief Start run never ending event loop.
	 *
	 * The loop will be terminated if any uncached exception threw. If
	 * the loop is already running, return immediately.
	 * 
	 */
	void run();

	/**
	 * @brief Add new event handler to event loop
	 * 
	 * @param handler The adding event handler
	 */
	void addEventHandlerToLoop(const Handler& handler);

	/**
	 * @brief Remove event handler from event loop.
	 * @note Remove handler from loop will not remove events from epoll,
	 * handlers need to clean it by themself.
	 * 
	 * @param handler The removing event handler
	 */
	void removeEventHandlerFromLoop(const Handler& handler);

	/// @brief Get poller object in this loop
	internal::epoll::Epoll *getPoll();

	/**
	 * @brief Get time wheel in this loop
	 * @note It could be nullptr if not time wheel was created
	 * 
	 */
	[[nodiscard]] internal::time::TimeWheel *getTimeWheel() const { return m_kickIdleConnectionWheel.get(); }

	/// @brief Runs method in event loop
	void runInLoop(std::function<void()> functor);

private:
	std::unique_ptr<EventLoopImpl> m_impl;
	std::shared_ptr<internal::handlers::RunInLoopHandler> m_runInLoop;
	std::unique_ptr<internal::time::TimeWheel> m_kickIdleConnectionWheel;
};
}

#endif //NETPP_EVENTLOOP_H
