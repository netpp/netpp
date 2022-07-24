//
// Created by gaojian on 2022/4/7.
//

#include <cassert>
#include "time/TickTimer.h"
#include "eventloop/EventLoopManager.h"
#include "time/TimeWheel.h"
#include "Application.h"
#include "eventloop/EventLoop.h"
#include "eventloop/EventLoopData.h"
#include "support/Util.h"

namespace netpp {
TickTimer::TickTimer(std::weak_ptr<TimeWheel> wheel)
		: m_wheel{std::move(wheel)}, m_wheelEntry{std::make_shared<WheelEntry>()}, m_running{false}
{
	m_wheelEntry->singleShot = true;
	m_wheelEntry->timeoutTick = 60;
	// if not specified wheel, run in main loop
	if (m_wheel.expired())
	{
		APPLICATION_INSTANCE_REQUIRED();
		auto manager = Application::loopManager();
		auto mainLoopData = manager->mainLoop()->loopData();
		m_wheel = mainLoopData->wheel;
	}
}

TickTimer::~TickTimer() = default;

void TickTimer::setOnTimeout(const std::function<void()> &callback)
{
	m_wheelEntry->callback = [this, callback] {
		m_running = false;
		callback();
	};
}

void TickTimer::setInterval(TimerInterval tick)
{
	m_wheelEntry->timeoutTick = tick;
}

void TickTimer::setSingleShot(bool singleShot)
{
	m_wheelEntry->singleShot = singleShot;
}

TimerInterval TickTimer::interval() const
{
	return m_wheelEntry->timeoutTick;
}

bool TickTimer::singleShot() const
{
	return m_wheelEntry->singleShot;
}

void TickTimer::start()
{
	auto wheel = m_wheel.lock();
	if (wheel)
	{
		m_running = true;
		wheel->addToWheel(m_wheelEntry);
	}
}

void TickTimer::restart()
{
	auto wheel = m_wheel.lock();
	if (wheel)
	{
		if (m_running)
			wheel->renew(m_wheelEntry);
		else
			start();
	}
}

void TickTimer::stop()
{
	auto wheel = m_wheel.lock();
	if (wheel && m_running)
	{
		m_running = false;
		wheel->removeFromWheel(m_wheelEntry);
	}
}
}
