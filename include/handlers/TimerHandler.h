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

namespace netpp::internal::handlers {
/**
 * @brief The TimerHandler handles timefd events, read event will triggered on timeout
 * 
 */
class TimerHandler : public epoll::EventHandler, public std::enable_shared_from_this<TimerHandler> {
	friend class time::Timer;
public:
	explicit TimerHandler(time::Timer *timer);
	~TimerHandler() override = default;

	/**
	 * @brief handle read events on time fd, triggered when
	 * 1.time fd timeout
	 * @note handlers will run only in EventLoop, NOT thread safe
	 */
	void handleRead() override;
	void handleWrite() override {}
	void handleError() override {}
	void handleClose() override {};

	/**
	 * @brief Remove timer handler from event loop
	 * 
	 */
	void remove();

private:
	time::Timer *_timer;
};
}

#endif //NETPP_EPOLLTIMER_H
