//
// Created by gaojian on 2020/8/6.
//

#include "internal/handlers/TimerHandler.h"
#include "time/Timer.h"
#include "internal/support/Log.h"
#include "EventLoop.h"
#include "internal/stub/IO.h"

namespace netpp::internal::handlers {
TimerHandler::TimerHandler(netpp::time::Timer *timer)
	: _timer{timer}
{}

void TimerHandler::handleIn()
{
	uint64_t triggerCount;
	if (stub::read(_timer->fd(), &triggerCount, sizeof(uint64_t)) != -1)
		_timer->onTimeOut();
}

void TimerHandler::remove()
{
	auto timer = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([timer]{
		timer->m_epollEvent->disable();
		timer->_loopThisHandlerLiveIn->removeEventHandlerFromLoop(timer);
	});
}
}
