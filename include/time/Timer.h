//
// Created by gaojian on 2020/8/9.
//

#ifndef NETPP_TIMER_H
#define NETPP_TIMER_H

#include <memory>
#include <functional>
#include "support/Types.h"

namespace netpp {
class EventLoop;
class TimerHandler;
/**
 * @brief Timer runs in EventLoop, but the ownership belongs to user,
 * the timer never be triggered if user delete the timer.
 * 
 * Timer is not thread, consider protect it if used in multithreading
 * 
 */
class Timer {
public:
	/**
	 * @brief Create a timer
	 * @param loop The timer runs in event loop, this param specify which loop this timer lives.
	 * By default nullptr passed, timer will try to find event loop in creating thread,
	 * if no event loop runs in current thread, then main loop is it.
	 */
	explicit Timer(EventLoop *loop = nullptr);
	~Timer();

	/**
	 * @brief Callback on timeout, the callback must NOT throw exception
	 * @param callback The callback on timeout
	 */
	void setOnTimeout(std::function<void()> callback) { m_callback = std::move(callback); }

	/**
	 * @brief Set timer trigger interval, by default, interval is 1000ms
	 * @param mSec The interval of timer, in milliseconds
	 */
	void setInterval(TimerInterval mSec);

	/**
	 * @brief Set timer is single shot, by default Timer is signle shot.
	 * If the timer is running, the single shot property will effect immediately.
	 * @param singleShot The timer should runs repeatedly
	 */
	void setSingleShot(bool singleShot);

	/// @brief Get timer interval
	[[nodiscard]] TimerInterval interval() const { return m_interval; }
	/// @brief Get is single shot
	[[nodiscard]] bool singleShot() const { return m_singleShot; }
	/// @brief Get is timer running
	[[nodiscard]] bool running() const { return m_running; }

	/// @brief Run timer
	void start();
	/// @brief Stop timer
	void stop();

private:
	void setAndRunTimer();

	TimerInterval m_interval;
	bool m_singleShot;
	bool m_running;	
	std::function<void()> m_callback;

	// event and handler
	std::shared_ptr<TimerHandler> m_handler;
};
}

#endif //NETPP_TIMER_H
