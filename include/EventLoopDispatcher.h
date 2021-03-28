//
// Created by gaojian on 2020/7/17.
//

#ifndef NETPP_EVENTLOOPDISPATCHER_H
#define NETPP_EVENTLOOPDISPATCHER_H

#include <memory>
#include <mutex>
#include <vector>

namespace netpp {
class EventLoop;
namespace support {
class ThreadPool;
}
}

namespace netpp {
/**
 * @brief The dispatcher for event loop,
 * netpp contains multiple reactors, one reactor per loop
 */
class EventLoopDispatcher {
public:
	/**
	 * @brief Construct a EventLoopDispatcher object
	 * 
	 * @param loopsCount				How many loops will be created
	 */
	explicit EventLoopDispatcher(unsigned loopsCount = 1);

	/**
	 * @brief Construct a EventLoopDispatcher object
	 * 
	 * @param loopsCount				How many loops will be created
	 * @param timeWheelRotateInterval	The time wheel tick interval
	 * @param timeWheelBucketCount		How many buckets the time wheel contains
	 */
	EventLoopDispatcher(unsigned loopsCount, unsigned timeWheelRotateInterval, unsigned timeWheelBucketCount);

	~EventLoopDispatcher();

	/**
	 * @brief Start run all event loop in this dispatcher
	 * @note One loop will run in caller thread, would block caller
	 * 
	 */
	void startLoop();

	/**
	 * @brief Dispatch an event loop
	 * 
	 * @return EventLoop*	The event loop dispatched
	 */
	EventLoop *dispatchEventLoop();

private:
	using EventLoopVector = std::vector<std::unique_ptr<EventLoop>>;
	std::mutex m_indexMutex;
	EventLoopVector::size_type m_dispatchIndex;	// guarded by m_indexMutex
	EventLoopVector m_loops;
	std::unique_ptr<support::ThreadPool> m_threadPool;
};
}

#endif //NETPP_EVENTLOOPDISPATCHER_H
