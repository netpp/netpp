//
// Created by gaojian on 2020/8/9.
//

#include "Timer.h"
#include "EventLoop.h"
#include "Log.h"

extern "C" {
#include <unistd.h>
}

using std::make_unique;
using std::make_shared;

namespace netpp {
Timer::Timer(EventLoop *loop)
	: m_interval(0), m_singleShot{true}, m_running{false}, m_timeOutCount{0}
{
	m_timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	m_timerSpec.it_value.tv_sec = 0;
	m_timerSpec.it_value.tv_nsec = 0;
	m_timerSpec.it_interval.tv_sec = 0;
	m_timerSpec.it_interval.tv_nsec = 0;
	SPDLOG_LOGGER_TRACE(logger, "Timer fd {}", m_timerFd);

	m_handler = make_shared<handlers::TimerHandler>(this);
	m_event = make_unique<epoll::EpollEvent>(loop->getPoll(), m_handler);
}

Timer::~Timer()
{
	::close(m_timerFd);
}

void Timer::onTimeOut()
{
	SPDLOG_LOGGER_TRACE(logger, "Timer {} triggered", m_timerFd);
	if (::read(m_timerFd, &m_timeOutCount, sizeof(uint64_t)) != -1 && m_callback)
	{
		m_callback();
	}
}

void Timer::setInterval(unsigned int msec)
{
	if (msec == 0)
		m_singleShot = true;
	m_interval = msec;
	m_timerSpec.it_interval.tv_sec = msec / 1000;
	m_timerSpec.it_interval.tv_nsec = (static_cast<long>(msec) % 1000) * 1000 * 1000;
}

void Timer::start()
{
	m_running = true;
	m_event->setEnableRead(true);
	// get first trigger time
	timespec now{0};
	if (::clock_gettime(CLOCK_MONOTONIC, &now) == -1)
		return;
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
	// start timer
	SPDLOG_LOGGER_TRACE(logger, "Start timer {} in {}.{}", m_timerFd, now.tv_sec, now.tv_nsec);
	if (::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &m_timerSpec, nullptr) == -1)
		return;
}

void Timer::stop()
{
	m_running = false;
	m_event->setEnableRead(false);
	m_timerSpec.it_value.tv_sec = 0;
	m_timerSpec.it_value.tv_nsec = 0;
	m_timerSpec.it_interval.tv_sec = 0;
	m_timerSpec.it_interval.tv_nsec = 0;
	if (::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &m_timerSpec, nullptr) == -1)
	{
	}
}
}