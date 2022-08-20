//
// Created by gaojian on 2022/4/7.
//

#ifndef NETPP_TICKTIMER_H
#define NETPP_TICKTIMER_H

#include <functional>
#include <memory>
#include "support/Types.h"
#include "TimerData.h"

namespace netpp {
class TimeWheel;
class EventLoop;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
/**
 * @brief The ticking timer, with limited duration and limited precision
 */
class TickTimer {
public:
	/**
	 * @brief Create a ticking timer, the timer start tick after constructed.
	 * @param wheel Where this timer ticks,
	 * by default nullptr passed, the timer runs in default wheel(the wheel in main event loop).
	 */
	explicit TickTimer(EventLoop *loop = nullptr);
	~TickTimer();

	/**
	 * @brief Calls when timeout
	 * @param callback The callback on timeout
	 */
	void setOnTimeout(const std::function<void()>& callback);

	/**
	 * @brief Set when will this timer be triggered, by default ticks 100 times
	 * @param tick after time wheel tick given times, timer timeout
	 */
	void setInterval(TimerInterval interval);

	/**
	 * @brief Set the timer should run repeatedly, by default TickTimer is single shot
	 * @param singleShot Should run timer after timeout
	 */
	void setSingleShot(bool singleShot);

	/**
	 * @brief Get timer interval
	 */
	[[nodiscard]] TimerInterval interval() const;

	/**
	 * @brief Get is single shot
	 */
	[[nodiscard]] bool singleShot() const;

	/// @brief Get is timer running
	[[nodiscard]] bool running() const { return m_running; }

	/**
	 * @brief Start run timer
	 */
	void start();

	/**
	 * @brief Start run timer
	 */
	void restart();

	/**
	 * @brief Stop timer
	 */
	void stop();

private:
	std::weak_ptr<TimeWheel> m_wheel;
	std::shared_ptr<TimerData> m_timerData;
	bool m_running;
};
}

#endif //NETPP_TICKTIMER_H
