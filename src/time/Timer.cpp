//
// Created by gaojian on 2020/8/9.
//

#include "time/Timer.h"
#include "EventLoop.h"
#include "Log.h"
#include "stub/IO.h"
extern "C" {
#include <unistd.h>
}

using std::make_unique;
using std::make_shared;

namespace netpp::time {
Timer::Timer(EventLoop *loop)
	: m_interval(0), m_singleShot{true}, m_running{false}, m_timeOutCount{0}
{
	m_timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	std::memset(&m_timerSpec, 0, sizeof(::itimerspec));
	SPDLOG_LOGGER_TRACE(logger, "Timer fd {}", m_timerFd);

	m_handler = make_shared<handlers::TimerHandler>(this);
	m_event = make_unique<epoll::EpollEvent>(loop->getPoll(), m_handler);
}

Timer::~Timer()
{
	if (stub::close(m_timerFd) == -1)
		SPDLOG_LOGGER_WARN(logger, "failed to close timer");
}

void Timer::onTimeOut()
{
	try {
		m_callback();
	} catch (...) {
		SPDLOG_LOGGER_CRITICAL(logger, "exception throwed while executing timeout method, stop");
		throw;
	}
}

void Timer::setInterval(unsigned int msec)
{
	m_interval = msec;
	if (m_running)	// if running affect immediately
		setTime();
	/*m_timerSpec.it_interval.tv_sec = msec / 1000;
	m_timerSpec.it_interval.tv_nsec = (static_cast<long>(msec) % 1000) * 1000 * 1000;*/
}

void Timer::start()
{
	if (!m_running)
	{
		m_running = true;
		// start timer
		m_event->setEnableRead(true);
		setTime();
	}
}

void Timer::stop()
{
	if (m_running)
	{
		m_running = false;
		std::memset(&m_timerSpec, 0, sizeof(::itimerspec));
		m_event->setEnableRead(false);
		// settime will always success
		::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &m_timerSpec, nullptr);
	}
}

void Timer::setTime()
{
	// get first trigger time
	timespec now{0};
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
	SPDLOG_LOGGER_TRACE(logger, "Start timer {} in {}.{}", m_timerFd, now.tv_sec, now.tv_nsec);
	
	// settime will always success
	::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &m_timerSpec, nullptr);
}
}
