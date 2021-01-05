//
// Created by gaojian on 2020/8/6.
//

#include "handlers/TimerHandler.h"
#include "Timer.h"

namespace netpp::handlers {
TimerHandler::TimerHandler(Timer *timer)
	: epoll::EventHandler(timer->fd()), _timer{timer}
{}

void TimerHandler::handleRead()
{
	_timer->onTimeOut();
}
}
