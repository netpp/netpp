//
// Created by gaojian on 2022/4/4.
//

#include "eventloop/EventLoopManager.h"
#include "Config.h"
#include "eventloop/EventLoop.h"
#include "internal/handlers/RunInLoopHandler.h"
#include "internal/handlers/SignalHandler.h"
#include "time/TimeWheel.h"

namespace netpp::eventloop {
EventLoopManager::EventLoopManager(const Config &config)
		: m_dispatchIndex{0}
{
	m_mainEventLoop = std::make_unique<MainEventLoopData>();
	m_loopData[m_mainEventLoop->eventLoop.get()] = dynamic_cast<EventLoopData *>(m_mainEventLoop.get());
	setUpEventLoop(m_mainEventLoop.get(), config, true);

	for (unsigned i = 0; i < config.eventLoopNumber - 1; ++i)
	{
		auto el = std::make_unique<EventLoopData>();
		m_loopData[el->eventLoop.get()] = dynamic_cast<EventLoopData *>(el.get());
		setUpEventLoop(el.get(), config, false);
		m_loops.emplace_back(std::move(el));
	}
}

EventLoop *EventLoopManager::dispatch()
{
	// TODO: load balance
	if (m_loops.empty())
		return m_mainEventLoop->eventLoop.get();
	std::lock_guard lck(m_indexMutex);
	if (++m_dispatchIndex >= m_loops.size())
		m_dispatchIndex = 0;
	return m_loops[m_dispatchIndex]->eventLoop.get();
}

std::vector<EventLoop *> EventLoopManager::loops() const
{
	std::vector<EventLoop *> loop;
	std::lock_guard lck(m_indexMutex);
	loop.emplace_back(m_mainEventLoop->eventLoop.get());
	for (auto &el : m_loops)
		loop.emplace_back(el->eventLoop.get());
	return loop;
}

EventLoop *EventLoopManager::mainLoop() const
{
	return m_mainEventLoop->eventLoop.get();
}

void EventLoopManager::startLoop()
{
	for (auto &l : m_loops)
	{
		std::thread t(&EventLoop::run, l->eventLoop.get());
		t.detach();
		m_loopsThreads.emplace_back(std::move(t));
	}
	m_mainEventLoop->eventLoop->run();
}

EventLoopData *EventLoopManager::getLoopData(EventLoop *loop) const
{
	auto it = m_loopData.find(loop);
	if (it != m_loopData.end())
		return it->second;
	return nullptr;
}

void EventLoopManager::setUpEventLoop(EventLoopData *loopData, const Config &config, bool mainEventLoop)
{
	EventLoop *loop = loopData->eventLoop.get();
	loopData->runInLoopHandler = netpp::internal::handlers::RunInLoopHandler::makeRunInLoopHandler(loop);

	if (mainEventLoop)
	{
		auto mainLoop = dynamic_cast<MainEventLoopData *>(loopData);
		if (config.tickTimer.enable)
			mainLoop->wheel = std::make_unique<netpp::time::TimeWheel>(loop, config.tickTimer.tickInterval, config.tickTimer.maxLength);
		if (config.enableDnsResolve)
		{}
		if (config.enableHandleSignal)
			mainLoop->signalHandler = netpp::internal::handlers::SignalHandler::makeSignalHandler(loop, config.eventHandler);
	}
}
}
