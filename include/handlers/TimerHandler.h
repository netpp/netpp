//
// Created by gaojian on 2020/8/6.
//

#ifndef NETPP_EPOLLTIMER_H
#define NETPP_EPOLLTIMER_H

#include <functional>
#include <memory>
#include "epoll/EventHandler.h"

namespace netpp::time {
class Timer;
}

namespace netpp::handlers {
class TimerHandler : public epoll::EventHandler {
public:
	explicit TimerHandler(time::Timer *timer);
	~TimerHandler() override = default;

	/// @brief time out
	void handleRead() override;
	void handleWrite() override {}
	void handleError() override {}
	void handleClose() override {}

private:
	time::Timer *_timer;
};
}

#endif //NETPP_EPOLLTIMER_H
