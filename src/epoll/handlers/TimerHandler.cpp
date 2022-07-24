//
// Created by gaojian on 2020/8/6.
//

#include "epoll/handlers/TimerHandler.h"
#include "time/Timer.h"
#include "eventloop/EventLoop.h"
#include "support/Log.h"
#include "internal/stub/IO.h"
#include <cstring>
#include <utility>
extern "C" {
#include <sys/timerfd.h>
}

namespace netpp {
TimerHandler::TimerHandler(EventLoop *loop)
		: EpollEventHandler(loop)
{
	m_timerFd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	LOG_TRACE("Timer fd {}", m_timerFd);
	activeEvents(EpollEv::IN);
}

TimerHandler::~TimerHandler()
{
	if (::close(m_timerFd) == -1)
		LOG_WARN("failed to close timer");
}

void TimerHandler::handleIn()
{
	uint64_t triggerCount;
	if (::read(m_timerFd, &triggerCount, sizeof(uint64_t)) != -1)
	{
		m_timeoutCallback();
	}
}

void TimerHandler::stopTimer() const
{
	::itimerspec timerSpec{};
	std::memset(&timerSpec, 0, sizeof(::itimerspec));
	// will always success
	::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &timerSpec, nullptr);
}

void TimerHandler::setIntervalAndRun(TimerInterval intervalInMSec, bool repeat, std::function<void()> callback)
{
	m_timeoutCallback = std::move(callback);
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
	::itimerspec timerSpec{};
	timerSpec.it_value.tv_sec = sec;
	timerSpec.it_value.tv_nsec = nSec;
	if (repeat)
	{
		timerSpec.it_interval.tv_sec = intervalInMSec / 1000;
		timerSpec.it_interval.tv_nsec = (static_cast<long>(intervalInMSec) % 1000) * 1000 * 1000;
	}
	else // if one shot
	{
		timerSpec.it_interval.tv_sec = 0;
		timerSpec.it_interval.tv_nsec = 0;
	}
	LOG_TRACE("Start timer {} in {}.{}", m_timerFd, now.tv_sec, now.tv_nsec);

	// timerfd_settime will always success
	::timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &timerSpec, nullptr);
}
}
