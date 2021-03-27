#ifndef NETPP_TIME_WHEEL_H
#define NETPP_TIME_WHEEL_H

#include <unordered_set>
#include <memory>
#include "time/Timer.h"
#include <string>

namespace netpp::internal::time {
class TimeWheelEntry {
	friend class TimeWheel;
public:
	explicit TimeWheelEntry(std::string name = "") : m_wheelName{std::move(name)}, m_indexInWheel{0} {}
	virtual ~TimeWheelEntry() = default;

	/**
	 * @brief To do what when time wheel triggered
	 * 
	 */
	virtual void onTimeout() = 0;
	virtual bool eraseOnTimeout() { return true; }

private:
	std::string m_wheelName;
	unsigned m_indexInWheel;
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
	TimeWheel(EventLoop *loop, unsigned tickInterval, unsigned bucketCount);
	void addToWheel(const std::shared_ptr<TimeWheelEntry>& entry);
	void removeFromWheel(const std::weak_ptr<TimeWheelEntry>& entry);
	void renew(const std::weak_ptr<TimeWheelEntry>& entry);

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
