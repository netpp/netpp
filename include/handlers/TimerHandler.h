//
// Created by gaojian on 2020/8/6.
//

#ifndef NETPP_EPOLLTIMER_H
#define NETPP_EPOLLTIMER_H

#include <functional>
#include <memory>
#include "epoll/EventHandler.h"

namespace netpp {
class Timer;
}

namespace netpp::handlers {
class TimerHandler : public epoll::EventHandler {
public:
	explicit TimerHandler(Timer *timer) noexcept;
	~TimerHandler() override = default;

	void handleRead() noexcept override;
	void handleWrite() noexcept override {}
	void handleError() noexcept override {}
	void handleClose() noexcept override {}

private:
	Timer *_timer;
};
}

#endif //NETPP_EPOLLTIMER_H
