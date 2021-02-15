//
// Created by gaojian on 2020/8/9.
//

#ifndef NETPP_TIMER_H
#define NETPP_TIMER_H

#include <memory>
#include "handlers/TimerHandler.h"
#include "epoll/EpollEvent.h"
extern "C" {
#include <sys/timerfd.h>
}

namespace netpp {
class EventLoop;
}

namespace netpp::time {
/**
 * @brief Timer runs in EventLoop, but the ownship does not belong to EventLoop,
 * if user does not own it, the timer will never triggred
 * 
 */
class Timer {
	friend class handlers::TimerHandler;
public:
	// TODO: remove param loop or set default value, user do not have to specify which event loop
	explicit Timer(EventLoop *loop);
	~Timer();

	/// @brief callback on timeout, the callback must NOT throw exception
	inline void setOnTimeout(std::function<void()> callback) { m_callback = callback; }
	/// @brief set timer trigger interval, by default, interval is 1000ms
	void setInterval(unsigned msec);
	/// @brief set timer is single shot, by default, the value is true
	void setSingleShot(bool singleShot) { m_singleShot = singleShot; }

	/// @brief get timer interval
	inline unsigned interval() const { return m_interval; }
	/// @brief get is signle shot
	inline bool singleShot() const { return m_singleShot; }
	/// @brief get is timer running
	inline bool running() const { return m_running; }

	/// @brief run timer
	void start();
	/// @brief stop timer
	void stop();

	/// @brief how many times the timer tirggered
	inline uint64_t triggeredCount() const { return m_timeOutCount; }

// for TimerHandler
private:
	/// @brief called on timeout, for internal use
	void onTimeOut();
	/// @brief get timerfd
	inline int fd() const { return m_timerFd; }

private:
	/// @brief set up timer, and start to run
	void setTimeAndRun();
	
	unsigned m_interval;
	bool m_singleShot;
	bool m_running;	
	std::function<void()> m_callback;

	// linux timer fd
	::itimerspec m_timerSpec;
	int m_timerFd;
	uint64_t m_timeOutCount;

	// event and hanlder
	std::shared_ptr<handlers::TimerHandler> m_handler;
	std::unique_ptr<epoll::EpollEvent> m_event;
};
}

#endif //NETPP_TIMER_H
