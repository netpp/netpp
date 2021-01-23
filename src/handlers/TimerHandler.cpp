//
// Created by gaojian on 2020/8/6.
//

#include "handlers/TimerHandler.h"
#include "Timer.h"
#include "Log.h"

namespace netpp::handlers {
TimerHandler::TimerHandler(Timer *timer) noexcept
	: epoll::EventHandler(timer->fd()), _timer{timer}
{}

void TimerHandler::handleRead() noexcept
{
	try {
		_timer->onTimeOut();
	} catch (...) {
		SPDLOG_LOGGER_CRITICAL(logger, "exception throwed while executing timeout method, stop");
		std::abort();
	}
}
}
