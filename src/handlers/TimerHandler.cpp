//
// Created by gaojian on 2020/8/6.
//

#include "handlers/TimerHandler.h"
#include "time/Timer.h"
#include "support/Log.h"
#include "EventLoop.h"

namespace netpp::internal::handlers {
TimerHandler::TimerHandler(time::Timer *timer)
	: _timer{timer}
{}

void TimerHandler::handleRead()
{
	uint64_t tirggeredCount;
	if (::read(_timer->fd(), &tirggeredCount, sizeof(uint64_t)) != -1)
		_timer->onTimeOut();
}

void TimerHandler::remove()
{
	auto timer = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([timer]{
		timer->m_epollEvent->deactiveEvents();
		timer->_loopThisHandlerLiveIn->removeEventHandlerFromLoop(timer);
	});
}
}
