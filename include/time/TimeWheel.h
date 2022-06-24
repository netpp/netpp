#ifndef NETPP_TIME_WHEEL_H
#define NETPP_TIME_WHEEL_H

#include <unordered_set>
#include <memory>
#include "time/Timer.h"
#include "support/Types.h"
#include <string>

namespace netpp {
struct WheelEntry {
private:
	friend class TimeWheel;
	/**
	 * @brief The index in TimeWheel, can be accessed by TimeWheel only
	 */
	TimerInterval wheelIndex = 0;
	bool expire = false;
public:
	/**
	 * @brief Can run this entry repeatedly
	 */
	bool singleShot = true;

	/**
	 * @brief The number of ticks this entry timeout
	 */
	TimerInterval timeoutTick = 0;

	/**
	 * @brief The callback when timeout
	 */
	std::function<void()> callback;
};

class TimeWheel {
public:
	/**
	 * @brief Construct a TimeWheel
	 * 
	 * @param loop			The event loop live in
	 * @param tickInterval	Wheel rotate interval, by milliseconds
	 * @param bucketCount	Contains n buckets
	 */
	TimeWheel(EventLoop *loop, TimerInterval tickInterval, TimerInterval bucketCount);

	void addToWheel(const std::shared_ptr<WheelEntry> &entry);
	void removeFromWheel(const std::shared_ptr<WheelEntry> &entry);
	void renew(const std::shared_ptr<WheelEntry> &entry);

private:
	void tick();

	Timer m_tickTimer;
	unsigned m_timeOutBucketIndex;

	std::vector<std::unordered_set<std::shared_ptr<WheelEntry>>> m_buckets;
};
}

#endif
