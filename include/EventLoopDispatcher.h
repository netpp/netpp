//
// Created by gaojian on 2020/7/17.
//

#ifndef NETPP_EVENTLOOPDISPATCHER_H
#define NETPP_EVENTLOOPDISPATCHER_H

#include "EventLoop.h"
#include <memory>
#include "support/ThreadPool.hpp"

namespace netpp {
/**
 * @brief The dispatcher for event loop,
 * netpp contains multipile reactors, one reactor per loop
 */
class EventLoopDispatcher {
public:
	/**
	 * @brief Construct a EventLoopDispatcher object
	 * 
	 * @param loopsCount				how many loops will be created
	 */
	explicit EventLoopDispatcher(unsigned loopsCount = 1);

	/**
	 * @brief Construct a EventLoopDispatcher object
	 * 
	 * @param loopsCount				how many loops will be created
	 * @param timeWheelRotateInterval	the time wheel tick interval
	 * @param timeWheelBucketCount		how many bockets the time wheel contains
	 */
	EventLoopDispatcher(unsigned loopsCount, unsigned timeWheelRotateInterval, unsigned timeWheelBucketCount);

	/**
	 * @brief start run all event loop in this dispatcher
	 * @note one loop will run in caller thread, would block caller
	 * 
	 */
	void startLoop();

	/**
	 * @brief dispatch an event loop
	 * 
	 * @return EventLoop* 
	 */
	EventLoop *dispatchEventLoop();

private:
	using EventLoopVector = std::vector<std::unique_ptr<EventLoop>>;
	EventLoopVector::size_type m_dispatchIndex;
	EventLoopVector m_loops;
	std::unique_ptr<support::ThreadPool> m_threadPool;
};
}

#endif //NETPP_EVENTLOOPDISPATCHER_H
