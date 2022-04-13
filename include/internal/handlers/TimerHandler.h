//
// Created by gaojian on 2020/8/6.
//

#ifndef NETPP_EPOLLTIMER_H
#define NETPP_EPOLLTIMER_H

#include <functional>
#include <memory>
#include "internal/epoll/EventHandler.h"
#include "time/TimerType.h"

namespace netpp::internal::handlers {
/**
 * @brief The TimerHandler handles time fd events, read event will triggered on timeout
 * 
 */
class TimerHandler : public epoll::EventHandler, public std::enable_shared_from_this<TimerHandler> {
public:
	explicit TimerHandler(eventloop::EventLoop *loop);
	~TimerHandler() override;

	/**
	 * @brief Remove timer handler from event loop
	 * 
	 */
	void remove();

	void stopTimer() const;
	void setIntervalAndRun(time::TimerInterval intervalInMSec, bool repeat, std::function<void()> callback);

	static std::shared_ptr<TimerHandler> makeTimerHandler(eventloop::EventLoop *loop);

protected:
	/**
	 * @brief handle read events on time fd, triggered when
	 * 1.time fd timeout
	 * @note handlers will run only in EventLoop, NOT thread safe
	 */
	void handleIn() override;

private:
	int m_timerFd;
	std::function<void()> m_timeoutCallback;
};
}

#endif //NETPP_EPOLLTIMER_H
