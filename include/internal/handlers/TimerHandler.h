//
// Created by gaojian on 2020/8/6.
//

#ifndef NETPP_EPOLLTIMER_H
#define NETPP_EPOLLTIMER_H

#include <functional>
#include <memory>
#include "internal/epoll/EventHandler.h"

namespace netpp::time {
class Timer;
}

namespace netpp::internal::handlers {
/**
 * @brief The TimerHandler handles time fd events, read event will triggered on timeout
 * 
 */
class TimerHandler : public epoll::EventHandler, public std::enable_shared_from_this<TimerHandler> {
	friend class netpp::time::Timer;
public:
	explicit TimerHandler(netpp::time::Timer *timer);
	~TimerHandler() override = default;

	/**
	 * @brief Remove timer handler from event loop
	 * 
	 */
	void remove();

protected:
	/**
	 * @brief handle read events on time fd, triggered when
	 * 1.time fd timeout
	 * @note handlers will run only in EventLoop, NOT thread safe
	 */
	void handleIn() override;

private:
	netpp::time::Timer *_timer;
};
}

#endif //NETPP_EPOLLTIMER_H
