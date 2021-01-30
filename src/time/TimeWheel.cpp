#include "time/TimeWheel.h"
#include "EventLoop.h"
#include "Log.h"

namespace netpp::time {
TimeWheel::TimeWheel(EventLoop *loop, unsigned tickInterval, unsigned bucketCount)
	: m_tickTimer(loop), m_buckets(bucketCount), m_timeOutBucketIndex(0)
{
	m_tickTimer.setOnTimeout(std::bind(&TimeWheel::tick, this));
	m_tickTimer.setInterval(tickInterval);
	m_tickTimer.setSingleShot(false);
	m_tickTimer.start();
}

void TimeWheel::addToWheel(std::shared_ptr<TimeWheelEntry> entry)
{
	// add entry to latest bucket
	unsigned latestIndex = (m_timeOutBucketIndex == 0) ? (m_buckets.size() - 1) : m_timeOutBucketIndex - 1;
	entry->m_indexInWheel = latestIndex;
	m_buckets[latestIndex].insert(entry);
}

void TimeWheel::removeFromWheel(std::weak_ptr<TimeWheelEntry> entry)
{
	auto item = entry.lock();
	if (item)
	{
		unsigned index = item->m_indexInWheel;
		auto &bucket = m_buckets[index];
		auto entryIt = bucket.find(item);
		if (entryIt != bucket.end())
			bucket.erase(entryIt);
	}
}

void TimeWheel::renew(std::weak_ptr<TimeWheelEntry> entry)
{
	auto item = entry.lock();
	if (item)
	{
		unsigned index = item->m_indexInWheel;
		unsigned nextIndex = ((index + 1) == m_buckets.size()) ? 0 : (index + 1);
		// refresh only when index is not in latest bucket
		if (nextIndex != m_timeOutBucketIndex)
		{
			// remove entry from previous bucket
			auto &bucket = m_buckets[index];
			auto entryIt = bucket.find(item);
			bucket.erase(entryIt);
			// add entry to latest bucket
			unsigned latestIndex = (m_timeOutBucketIndex == 0) ? (m_buckets.size() - 1) : m_timeOutBucketIndex - 1;
			item->m_indexInWheel = latestIndex;
			m_buckets[latestIndex].insert(item);
		}
	}
}

void TimeWheel::tick()
{
	++m_timeOutBucketIndex;
	if (m_timeOutBucketIndex == m_buckets.size())
		m_timeOutBucketIndex = 0;
	auto &bucket = m_buckets[m_timeOutBucketIndex];
	for (auto entry : bucket)
		entry->onTimeout();
	bucket.clear();
}
}
