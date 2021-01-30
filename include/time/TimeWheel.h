#ifndef NETPP_TIME_WHEEL_H
#define NETPP_TIME_WHEEL_H

#include <unordered_set>
#include <memory>
#include "Timer.h"

namespace netpp::time {

class TimeWheelEntry {
	friend class TimeWheel;
public:
	TimeWheelEntry() : m_indexInWheel{0} {}
	virtual ~TimeWheelEntry() = default;

	virtual void onTimeout() = 0;

private:
	unsigned m_indexInWheel;
};

class TimeWheel {
public:
	using Bucket = std::unordered_set<std::shared_ptr<TimeWheelEntry>>;

	/**
	 * @brief Construct a new Time Wheel object
	 * 
	 * @param loop			The event loop live in
	 * @param tickInterval	Wheel rotate interval, by milliseconds
	 * @param bucketCount	Contains n buckets
	 */
	TimeWheel(EventLoop *loop, unsigned tickInterval, unsigned bucketCount);
	void addToWheel(std::shared_ptr<TimeWheelEntry> entry);
	void removeFromWheel(std::weak_ptr<TimeWheelEntry> entry);
	void renew(std::weak_ptr<TimeWheelEntry> entry);

private:
	void tick();

	Timer m_tickTimer;
	unsigned m_timeOutBucketIndex;
	std::vector<Bucket> m_buckets;
};
}

#endif
