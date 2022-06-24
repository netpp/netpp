//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EVENTLOOP_H
#define NETPP_EVENTLOOP_H

#include <functional>
#include <memory>
#include <unordered_set>
#include <mutex>

namespace netpp {
struct EventLoopData;
class EpollEventHandler;
class Epoll;
/**
 * @brief The event loop wait/dispatch/handle events.
 *
 * Event loop runs only in one thread, and a thread can only run one event loop
 */
class EventLoop {
	friend class EventLoopFactory;
public:
	/**
	 * @brief Handler type
	 */
	using Handler = std::shared_ptr<EpollEventHandler>;

	/**
	 * @brief Default EventLoop, will not create time wheel
	 * @throw ResourceLimitException
	 * 1. file descriptors limit
	 * 2. no more memory
	 */
	EventLoop();

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
	void addEventHandlerToLoop(const Handler &handler);

	/**
	 * @brief Remove event handler from event loop.
	 * @note Remove handler from loop will not remove events from epoll,
	 * handlers need to clean it by themself.
	 * 
	 * @param handler The removing event handler
	 */
	void removeEventHandlerFromLoop(const Handler &handler);

	/// @brief Get poller object in this loop
	Epoll *getPoll();

	/**
	 * @brief Get the event loop running on current thread
	 * @return The event loop running, if no event loop runs in this thread, nullptr returned
	 */
	static EventLoop *thisLoop();
	void runInLoop(std::function<void()> task);

	const EventLoopData *loopData() const { return m_loopData.get(); }

private:
	std::atomic_flag m_loopRunning;
	std::unique_ptr<Epoll> m_poll;

	std::mutex m_handlersMutex;    // guard m_handlers
	std::unordered_set<EventLoop::Handler> m_handlers;    // epoll events handlers

	std::unique_ptr<EventLoopData> m_loopData;
};
}

#endif //NETPP_EVENTLOOP_H
