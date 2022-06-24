//
// Created by gaojian on 2020/8/6.
//

#ifndef NETPP_EPOLLTIMER_H
#define NETPP_EPOLLTIMER_H

#include <functional>
#include <memory>
#include "epoll/EpollEventHandler.h"
#include "support/Types.h"

namespace netpp {
/**
 * @brief The TimerHandler handles time fd events, read event will triggered on timeout
 * 
 */
class TimerHandler : public EpollEventHandler, public std::enable_shared_from_this<TimerHandler> {
public:
	explicit TimerHandler(EventLoop *loop);
	~TimerHandler() override;

	/**
	 * @brief Remove timer handler from event loop
	 * 
	 */
	void remove();

	void stopTimer() const;
	void setIntervalAndRun(TimerInterval intervalInMSec, bool repeat, std::function<void()> callback);

protected:
	/**
	 * @brief handle read events on time fd, triggered when
	 * 1.time fd timeout
	 * @note handlers will run only in EventLoop, NOT thread safe
	 */
	void handleIn() override;

	int fileDescriptor() const override { return m_timerFd; }

private:
	int m_timerFd;
	std::function<void()> m_timeoutCallback;
};
}

#endif //NETPP_EPOLLTIMER_H
