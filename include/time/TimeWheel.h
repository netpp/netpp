#ifndef NETPP_TIME_WHEEL_H
#define NETPP_TIME_WHEEL_H

#include <unordered_set>
#include <memory>
#include "Timer.h"
#include <string>

namespace netpp::internal::time {
// TODO: maybe move TimeWheel to internal namespace
class TimeWheelEntry {
	friend class TimeWheel;
public:
	TimeWheelEntry(std::string name = "") : m_wheelName{name}, m_indexInWheel{0} {}
	virtual ~TimeWheelEntry() = default;

	virtual void onTimeout() = 0;

private:
	std::string m_wheelName;
	unsigned m_indexInWheel;
};

class TimeWheel {
public:
	/**
	 * @brief Construct a new Time Wheel object
	 * 
	 * @param loop			The event loop live in
	 * @param tickInterval	Wheel rotate interval, by milliseconds
	 * @param bucketCount	Contains n buckets
	 */
	TimeWheel(EventLoop *loop, unsigned tickInterval, unsigned bucketCount);
	void addToWheel(std::shared_ptr<TimeWheelEntry> entry);
	// void removeFromWheel(std::weak_ptr<TimeWheelEntry> entry);
	void renew(std::weak_ptr<TimeWheelEntry> entry);

private:
	void tick();

	netpp::time::Timer m_tickTimer;
	unsigned m_bucketSize;
	unsigned m_timeOutBucketIndex;

	using Bucket = std::unordered_set<std::shared_ptr<TimeWheelEntry>>;
	std::vector<Bucket> m_buckets;
};
}

#endif
