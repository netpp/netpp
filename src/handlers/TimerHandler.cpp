//
// Created by gaojian on 2020/8/6.
//

#include "handlers/TimerHandler.h"
#include "time/Timer.h"
#include "Log.h"

namespace netpp::handlers {
TimerHandler::TimerHandler(time::Timer *timer)
	: _timer{timer}
{}

void TimerHandler::handleRead()
{
	uint64_t tirggeredCount;
	if (::read(_timer->fd(), &tirggeredCount, sizeof(uint64_t)) != -1)
		_timer->onTimeOut();
}
}
