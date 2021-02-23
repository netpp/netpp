//
// Created by gaojian on 2020/8/9.
//

#include "time/Timer.h"
#include "EventLoop.h"
#include "support/Log.h"
#include "stub/IO.h"
#include "handlers/TimerHandler.h"
#include "epoll/EpollEvent.h"
extern "C" {
#include <unistd.h>
}

using std::make_unique;
using std::make_shared;

namespace netpp::time {
Timer::Timer(EventLoop *loop)
	: m_interval(1000), m_singleShot{true}, m_running{false}, m_timeOutCount{0}
{
	m_timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	std::memset(&m_timerSpec, 0, sizeof(::itimerspec));
	LOG_TRACE("Timer fd {}", m_timerFd);

	m_handler = make_shared<internal::handlers::TimerHandler>(this);
	m_handler->m_epollEvent = make_unique<internal::epoll::EpollEvent>(loop->getPoll(), m_handler, m_timerFd);
	m_handler->m_epollEvent->setEnableRead(true);
	m_handler->_loopThisHandlerLiveIn = loop;
}

Timer::~Timer()
{
	if (internal::stub::close(m_timerFd) == -1)
		LOG_WARN("failed to close timer");
	m_handler->remove();
}

void Timer::onTimeOut()
{
	try
	{
		m_callback();
	}
	catch (...)
	{
		LOG_CRITICAL("exception throwed while executing timeout method, stop");
		throw;
	}
}

void Timer::setInterval(unsigned int msec)
{
	m_interval = msec;
	if (m_running)	// if running, affect immediately
		setTimeAndRun();
	/*m_timerSpec.it_interval.tv_sec = msec / 1000;
	m_timerSpec.it_interval.tv_nsec = (static_cast<long>(msec) % 1000) * 1000 * 1000;*/
}

void Timer::start()
{
	if (!m_running)
	{
		m_running = true;
		// start timer
		setTimeAndRun();
	}
}

void Timer::stop()
{
	if (m_running)
	{
		m_running = false;
		std::memset(&m_timerSpec, 0, sizeof(::itimerspec));
		// settime will always success
		::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &m_timerSpec, nullptr);
	}
}

void Timer::setTimeAndRun()
{
	// get first trigger time
	timespec now;
	// can ignore gettime failed
	::clock_gettime(CLOCK_MONOTONIC, &now);
	unsigned sec = now.tv_sec + m_interval / 1000;
	long nsec = now.tv_nsec + (static_cast<long>(m_interval) % 1000) * 1000 * 1000;
	if (nsec > 1000000000)
	{
		sec += 1;
		nsec -= 1000000000;
	}
	m_timerSpec.it_value.tv_sec = sec;
	m_timerSpec.it_value.tv_nsec = nsec;
	// if one shot
	if (m_singleShot)
	{
		m_timerSpec.it_interval.tv_sec = 0;
		m_timerSpec.it_interval.tv_nsec = 0;
	}
	else
	{
		m_timerSpec.it_interval.tv_sec = m_interval / 1000;
		m_timerSpec.it_interval.tv_nsec = (static_cast<long>(m_interval) % 1000) * 1000 * 1000;
	}
	LOG_TRACE("Start timer {} in {}.{}", m_timerFd, now.tv_sec, now.tv_nsec);
	
	// settime will always success
	::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &m_timerSpec, nullptr);
}
}
