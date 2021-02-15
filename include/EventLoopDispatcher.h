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
	// TODO: maybe move params into config class
	explicit EventLoopDispatcher(unsigned loopsCount = 1);
	EventLoopDispatcher(unsigned loopsCount, unsigned timeWheelRotateInterval, unsigned timeWheelBucketCount);

	void startLoop();

	EventLoop *dispatchEventLoop();

private:
	using EventLoopVector = std::vector<std::unique_ptr<EventLoop>>;
	EventLoopVector::size_type m_dispatchIndex;
	EventLoopVector m_loops;
	std::unique_ptr<support::ThreadPool> m_threadPool;
};
}

#endif //NETPP_EVENTLOOPDISPATCHER_H
