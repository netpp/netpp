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
TickTimer::TickTimer(EventLoop *loop)
	: m_timerData{std::make_unique<TimeWheel::WheelEntryData>()}, m_running{false}
{
	bool useMainLoop = true;
	if (loop)
	{
		auto wheel = loop->loopData()->wheel;
		if (wheel)
		{
			useMainLoop = false;
			m_wheel = wheel;
		}
	}
	if (useMainLoop)
	{
		APPLICATION_INSTANCE_REQUIRED();
		auto manager = Application::loopManager();
		auto mainLoopData = manager->mainLoop()->loopData();
		m_wheel = mainLoopData->wheel;
	}
}

TickTimer::~TickTimer()
{
	stop();
}

void TickTimer::setOnTimeout(const std::function<void()> &callback)
{
	m_timerData->callback = [this, callback] {
		if (m_timerData->singleShot)
			m_running = false;
		if (callback)
			callback();
	};
}

void TickTimer::setInterval(TimerInterval interval)
{
	m_timerData->interval = interval;
}

void TickTimer::setSingleShot(bool singleShot)
{
	m_timerData->singleShot = singleShot;
}

TimerInterval TickTimer::interval() const
{
	return m_timerData->interval;
}

bool TickTimer::singleShot() const
{
	return m_timerData->singleShot;
}

void TickTimer::start()
{
	auto wheel = m_wheel.lock();
	if (wheel)
	{
		m_running = true;
		wheel->addToWheel(this, dynamic_cast<const TimeWheel::WheelEntryData &>(*m_timerData));
	}
}

void TickTimer::restart()
{
	auto wheel = m_wheel.lock();
	if (wheel)
	{
		if (m_running)
			wheel->renew(dynamic_cast<const TimeWheel::WheelEntryData &>(*m_timerData));
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
		wheel->removeFromWheel(dynamic_cast<const TimeWheel::WheelEntryData &>(*m_timerData));
	}
}
}
