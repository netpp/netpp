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
namespace epoll {
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
	~EventLoop();

	[[noreturn]] void run();
	void addEventHandlerToLoop(std::shared_ptr<epoll::EventHandler> handler);
	void removeEventHandlerFromLoop(std::shared_ptr<epoll::EventHandler> handler);

	/**
	 * @brief return this thread's event loop
	 * @note only avaiable AFTER loop start
	 */
	static inline EventLoop *thisLoop() { return _thisThreadLoop; }
	inline epoll::Epoll *getPoll() { return &m_poll; }
	time::TimeWheel *getTimeWheel() { return m_kickIdleConnectionWheel.get(); }

	/// @brief runs method in event loop
	void runInLoop(std::function<void()> functor);

private:
	static thread_local EventLoop *_thisThreadLoop;
	epoll::Epoll m_poll;
	std::unordered_set<std::shared_ptr<epoll::EventHandler>> m_handlers;	// epoll events handlers

	std::mutex m_functorMutex;							// guard m_pendingFuns
	std::vector<std::function<void()>> m_pendingFuns;	// methods run in loop

	std::unique_ptr<time::TimeWheel> m_kickIdleConnectionWheel;
};
}

#endif //NETPP_EVENTLOOP_H
