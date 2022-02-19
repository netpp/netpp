//
// Created by gaojian on 2020/8/9.
//

#include "time/Timer.h"
#include "EventLoop.h"
#include "internal/support/Log.h"
#include "internal/handlers/TimerHandler.h"
#include "internal/epoll/EpollEvent.h"
#include "internal/stub/IO.h"
#include <cstring>
extern "C" {
#include <sys/timerfd.h>
}

using std::make_unique;
using std::make_shared;

namespace netpp::time {
class TimerImpl {
public:
	TimerImpl();
	~TimerImpl();

	void stop();
	/**
	 * @brief Set timer interval to 0, not effect start value
	 */
	void changeIntervalTo(unsigned int intervalInMSec);

	void triggerOnceIn(unsigned int startInMSec);

	/**
	 * @brief Set timer interval, effect both start value and interval
	 * @param intervalInMSec	Interval in milliseconds
	 */
	void runRepeatedly(unsigned int intervalInMSec);

	[[nodiscard]] int timerFd() const { return m_timerFd; }

private:
	void setAndRun(unsigned int intervalInMSec, bool repeat);

	// linux timer fd
	::itimerspec m_timerSpec;
	int m_timerFd;
};

Timer::Timer(EventLoop *loop)
	: m_interval(1000), m_singleShot{true}, m_running{false}
{
	m_impl = make_unique<time::TimerImpl>();

	m_handler = std::make_shared<internal::handlers::TimerHandler>(this);
	m_handler->m_epollEvent = make_unique<internal::epoll::EpollEvent>(loop->getPoll(), m_handler, m_impl->timerFd());
	m_handler->m_epollEvent->active(internal::epoll::EpollEv::IN);
	m_handler->_loopThisHandlerLiveIn = loop;
}

Timer::~Timer()
{
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
		LOG_CRITICAL("exception threw while executing timeout method, stop");
		throw;
	}
}

int Timer::fd() const
{
	return m_impl->timerFd();
}

void Timer::setInterval(unsigned int mSec)
{
	m_interval = mSec;
	if (m_running)	// if running, affect immediately
	{
		if (m_singleShot)
			m_impl->triggerOnceIn(mSec);
		else
			m_impl->runRepeatedly(mSec);
	}
}

void Timer::setSingleShot(bool singleShot)
{
	if (m_singleShot == singleShot)
		return;
	m_singleShot = singleShot;
	if (running())
	{
		// if set to single shot, set interval to 0
		if (m_singleShot)
		{
			m_impl->triggerOnceIn(0);
			LOG_TRACE("Start timer to single shot");
		}
		else	// set interval
		{
			m_impl->runRepeatedly(m_interval);
			LOG_TRACE("Start timer run repeatedly");
		}
	}
}

void Timer::start()
{
	if (!m_running)
	{
		m_running = true;
		// start timer
		if (m_singleShot)
			m_impl->triggerOnceIn(m_interval);
		else
			m_impl->runRepeatedly(m_interval);
	}
}

void Timer::stop()
{
	if (m_running)
	{
		m_running = false;
		m_impl->stop();
	}
}

TimerImpl::TimerImpl()
	: m_timerSpec{}
{
	m_timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	std::memset(&m_timerSpec, 0, sizeof(::itimerspec));
	LOG_TRACE("Timer fd {}", m_timerFd);
}

TimerImpl::~TimerImpl()
{
	if (internal::stub::close(m_timerFd) == -1)
		LOG_WARN("failed to close timer");
}

void TimerImpl::stop()
{
	std::memset(&m_timerSpec, 0, sizeof(::itimerspec));
	// will always success
	::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &m_timerSpec, nullptr);
}

void TimerImpl::changeIntervalTo(unsigned int intervalInMSec)
{
	m_timerSpec.it_interval.tv_sec = intervalInMSec / 1000;
	m_timerSpec.it_interval.tv_nsec = (static_cast<long>(intervalInMSec) % 1000) * 1000 * 1000;
	// will always success
	::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &m_timerSpec, nullptr);
}

void TimerImpl::triggerOnceIn(unsigned int startInMSec)
{
	setAndRun(startInMSec, false);
}

void TimerImpl::runRepeatedly(unsigned int intervalInMSec)
{
	setAndRun(intervalInMSec, true);
}

void TimerImpl::setAndRun(unsigned int intervalInMSec, bool repeat)
{
	// get first trigger time
	::timespec now{};
	// can ignore clock_gettime failed
	::clock_gettime(CLOCK_MONOTONIC, &now);
	::time_t sec = now.tv_sec + intervalInMSec / 1000;
	long nSec = now.tv_nsec + (static_cast<long>(intervalInMSec) % 1000) * 1000 * 1000;
	if (nSec > 1000000000)
	{
		sec += 1;
		nSec -= 1000000000;
	}
	m_timerSpec.it_value.tv_sec = sec;
	m_timerSpec.it_value.tv_nsec = nSec;
	if (repeat)
	{
		m_timerSpec.it_interval.tv_sec = intervalInMSec / 1000;
		m_timerSpec.it_interval.tv_nsec = (static_cast<long>(intervalInMSec) % 1000) * 1000 * 1000;
	}
	else // if one shot
	{
		m_timerSpec.it_interval.tv_sec = 0;
		m_timerSpec.it_interval.tv_nsec = 0;
	}
	LOG_TRACE("Start timer {} in {}.{}", m_timerFd, now.tv_sec, now.tv_nsec);

	// timerfd_settime will always success
	::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &m_timerSpec, nullptr);
}
}
