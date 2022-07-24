//
// Created by gaojian on 2022/4/4.
//

#include "eventloop/EventLoopManager.h"
#include "eventloop/EventLoop.h"
#include "eventloop/EventLoopFactory.h"

namespace netpp {
EventLoopManager::EventLoopManager(unsigned loopsCount)
		: m_dispatchIndex{0}
{
	m_mainEventLoop = EventLoopFactory::makeEventLoop(true, true);

	for (unsigned i = 0; i < loopsCount - 1; ++i)
	{
		auto el = EventLoopFactory::makeEventLoop(true, false);
		m_loops.emplace_back(std::move(el));
	}
}

EventLoopManager::~EventLoopManager()
{
	// todo: let loop quit
	for (auto &l : m_loops)
		l->runInLoop(std::function<void()>());	// wake up loop, so we can quit
	for (auto &t : m_loopsThreads)
		t.join();
}

EventLoop *EventLoopManager::dispatch()
{
	// TODO: load balance
	if (m_loops.empty())
		return m_mainEventLoop.get();
	std::lock_guard lck(m_indexMutex);
	if (++m_dispatchIndex >= m_loops.size())
		m_dispatchIndex = 0;
	return m_loops[m_dispatchIndex].get();
}

std::vector<EventLoop *> EventLoopManager::loops() const
{
	std::vector<EventLoop *> loop;
	std::lock_guard lck(m_indexMutex);
	loop.emplace_back(m_mainEventLoop.get());
	for (auto &el : m_loops)
		loop.emplace_back(el.get());
	return loop;
}

EventLoop *EventLoopManager::mainLoop() const
{
	return m_mainEventLoop.get();
}

void EventLoopManager::startLoop()
{
	for (auto &l : m_loops)
	{
		std::thread t(&EventLoop::run, l.get());
		t.detach();
		m_loopsThreads.emplace_back(std::move(t));
	}
	m_mainEventLoop->run();
}
}
