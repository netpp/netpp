//
// Created by gaojian on 2020/7/17.
//

#include "EventLoopDispatcher.h"
#include "support/ThreadPool.hpp"
#include "support/Log.h"

namespace netpp {
EventLoopDispatcher::EventLoopDispatcher(unsigned loopsCount)
	: m_dispatchIndex{0}
{
	// one loop will run in main thread
	if (loopsCount > 1)
		m_threadPool = std::make_unique<support::ThreadPool>(loopsCount - 1);
	else
		m_threadPool = nullptr;
	
	for (unsigned i = 0; i < loopsCount; ++i)
		m_loops.emplace_back(std::make_unique<EventLoop>());
}

EventLoopDispatcher::EventLoopDispatcher(unsigned loopsCount, unsigned timeWheelRotateInterval, unsigned timeWheelBucketCount)
	: m_dispatchIndex{0}
{
	// one loop will run in main thread
	if (loopsCount > 1)
		m_threadPool = std::make_unique<support::ThreadPool>(loopsCount - 1);
	else
		m_threadPool = nullptr;
	
	for (unsigned i = 0; i < loopsCount; ++i)
		m_loops.emplace_back(std::make_unique<EventLoop>(timeWheelRotateInterval, timeWheelBucketCount));
}

EventLoop *EventLoopDispatcher::dispatchEventLoop()
{
	// TODO: load balance
	if (m_loops.size() == 0)
		return nullptr;
	std::lock_guard lck(m_indexMutex);
	if (++m_dispatchIndex >= m_loops.size())
		m_dispatchIndex = 0;
	return m_loops[m_dispatchIndex].get();
}

void EventLoopDispatcher::startLoop()
{
	internal::initLogger();
	if (m_threadPool)
	{
		for (EventLoopVector::size_type i = 1; i < m_loops.size(); ++i)
			m_threadPool->run(std::bind(&EventLoop::run, m_loops[i].get()));
		m_threadPool->start();
	}
	if (m_loops.size() != 0)
		m_loops[0]->run();
}
}
