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

void TimerHandler::handleClose()
{
	// this will not triggered by epoll, used to close timer
	m_epollEvent->disableEvents();
	// extern TcpConnection life after remove
	volatile auto externLife = shared_from_this();
	// FIXME: may called from other theads
	EventLoop::thisLoop()->removeEventHandlerFromLoop(shared_from_this());
}

void TimerHandler::setEnabled(bool enable)
{
	m_epollEvent->setEnableRead(enable);
}
}
