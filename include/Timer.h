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

namespace netpp {
class Timer {
public:
	explicit Timer(EventLoop *loop);
	~Timer();

	inline void setOnTimeout(std::function<void()> callback) { m_callback = callback; }
	void onTimeOut();
	void setInterval(unsigned msec);
	void setSingleShot(bool singleShot) { m_singleShot = singleShot; }

	inline int fd() const { return m_timerFd; }
	inline unsigned interval() const { return m_interval; }
	inline bool singleShot() const { return m_singleShot; }
	inline bool running() const { return m_running; }

	void start();
	void stop();

	inline uint64_t triggeredCount() const { return m_timeOutCount; }

private:
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
