//
// Created by gaojian on 2020/8/9.
//

#ifndef NETPP_TIMER_H
#define NETPP_TIMER_H

#include <memory>
#include <functional>

namespace netpp {
class EventLoop;
namespace internal::handlers {
class TimerHandler;
}
}

namespace netpp::time {
class TimerImpl;
/**
 * @brief Timer runs in EventLoop, but the ownership does not belong to EventLoop,
 * if user does not own it, the timer will never triggered.
 * 
 * Timer is not thread, consider protect it if used in multithreading
 * 
 */
class Timer {
	friend class internal::handlers::TimerHandler;
public:
	// TODO: remove param loop or set default value, user do not have to specify which event loop
	explicit Timer(EventLoop *loop);
	~Timer();

	/// @brief Callback on timeout, the callback must NOT throw exception
	inline void setOnTimeout(std::function<void()> callback) { m_callback = std::move(callback); }
	/// @brief Set timer trigger interval, by default, interval is 1000ms
	void setInterval(unsigned mSec);
	
	/**
	 * @brief Set timer is single shot, by default, the value is true.
	 * If the timer is running, the single shot property will effect immediately.
	 * 
	 */
	void setSingleShot(bool singleShot);

	/// @brief Get timer interval
	[[nodiscard]] inline unsigned interval() const { return m_interval; }
	/// @brief Get is single shot
	[[nodiscard]] inline bool singleShot() const { return m_singleShot; }
	/// @brief Get is timer running
	[[nodiscard]] inline bool running() const { return m_running; }

	/// @brief Run timer
	void start();
	/// @brief Stop timer
	void stop();

// for TimerHandler
private:
	/// @brief Called on timeout, for internal use
	void onTimeOut();
	/// @brief Get timer fd
	[[nodiscard]] int fd() const;

private:
	unsigned m_interval;
	bool m_singleShot;
	bool m_running;	
	std::function<void()> m_callback;

	std::unique_ptr<time::TimerImpl> m_impl;
	// event and handler
	std::shared_ptr<internal::handlers::TimerHandler> m_handler;
};
}

#endif //NETPP_TIMER_H
