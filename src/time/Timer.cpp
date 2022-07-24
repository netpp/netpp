//
// Created by gaojian on 2020/8/9.
//

#include <cassert>
#include "time/Timer.h"
#include "eventloop/EventLoop.h"
#include "support/Log.h"
#include "epoll/handlers/TimerHandler.h"
#include "Application.h"
#include "eventloop/EventLoopManager.h"
#include "support/Util.h"

using std::make_unique;
using std::make_shared;

namespace netpp {
Timer::Timer(EventLoop *loop)
	: m_interval(1000), m_singleShot{true}, m_running{false}
{
	// timer runs in creating thread
	if (!loop)
		loop = EventLoop::thisLoop();
	// timer runs in main loop
	if (!loop)
	{
		APPLICATION_INSTANCE_REQUIRED();
		loop = Application::loopManager()->mainLoop();
	}
	m_handler = make_shared<TimerHandler>(loop);
	m_timerLoop = loop;
}

Timer::~Timer()
{
	m_timerLoop->removeEventHandlerFromLoop(m_handler);
}

void Timer::setInterval(TimerInterval mSec)
{
	m_interval = mSec;
	if (m_running)	// if running, affect immediately
	{
		setAndRunTimer();
	}
}

void Timer::setSingleShot(bool singleShot)
{
	if (m_singleShot == singleShot)
		return;
	m_singleShot = singleShot;
	if (running())
	{
		setAndRunTimer();
	}
}

void Timer::start()
{
	if (!m_running)
	{
		// start timer
		setAndRunTimer();
	}
}

void Timer::stop()
{
	if (m_running)
	{
		m_running = false;
		m_handler->stopTimer();
	}
}

void Timer::setAndRunTimer()
{
	auto callback = [this]{
		if (m_singleShot)
			m_running = false;
		if (m_callback)
		{
			try
			{
				m_callback();
			}
			catch (...)
			{
				LOG_CRITICAL("exception threw while executing timeout method, stop");
				throw;
			}
		}
	};
	m_running = true;
	if (m_singleShot)
		m_handler->setIntervalAndRun(m_interval, false, callback);
	else
		m_handler->setIntervalAndRun(m_interval, true, callback);
}
}
