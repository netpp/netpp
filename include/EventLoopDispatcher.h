//
// Created by gaojian on 2020/7/17.
//

#ifndef NETPP_EVENTLOOPDISPATCHER_H
#define NETPP_EVENTLOOPDISPATCHER_H

#include "EventLoop.h"
#include <memory>
#include "support/ThreadPool.hpp"

namespace netpp {
class EventLoopDispatcher {
public:
	explicit EventLoopDispatcher(unsigned loopsCount = 1);
	void startLoop();

	EventLoop *dispatchEventLoop();

private:
	using EventLoopVector = std::vector<std::unique_ptr<EventLoop>>;
	EventLoopVector::size_type m_dispatchIndex;
	EventLoopVector m_loops;
	support::ThreadPool m_threadPool;
};
}

#endif //NETPP_EVENTLOOPDISPATCHER_H
