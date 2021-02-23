//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EVENTLOOP_H
#define NETPP_EVENTLOOP_H

#include "epoll/Epoll.h"
#include "time/TimeWheel.h"
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
	EventLoop() = default;

	/**
	 * @brief Construct an EventLoop with timewheel
	 * 
	 * @param tickInterval	Wheel rotate interval, by milliseconds
	 * @param bucketCount	Contains n buckets
	 */
	EventLoop(unsigned tickInterval, unsigned bucketCount);
	~EventLoop() = default;

	[[noreturn]] void run();
	void addEventHandlerToLoop(std::shared_ptr<internal::epoll::EventHandler> handler);
	void removeEventHandlerFromLoop(std::shared_ptr<internal::epoll::EventHandler> handler);

	inline internal::epoll::Epoll *getPoll() { return &m_poll; }
	internal::time::TimeWheel *getTimeWheel() { return m_kickIdleConnectionWheel.get(); }

	/// @brief runs method in event loop
	void runInLoop(std::function<void()> functor);

private:
	internal::epoll::Epoll m_poll;
	std::unordered_set<std::shared_ptr<internal::epoll::EventHandler>> m_handlers;	// epoll events handlers

	std::mutex m_functorMutex;							// guard m_pendingFuns
	std::vector<std::function<void()>> m_pendingFuns;	// methods run in loop

	std::unique_ptr<internal::time::TimeWheel> m_kickIdleConnectionWheel;
};
}

#endif //NETPP_EVENTLOOP_H
