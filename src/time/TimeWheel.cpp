#include "time/TimeWheel.h"
#include "eventloop/EventLoop.h"
#include "support/Log.h"

namespace {
/**
 * @brief Cast time interval into wheel's bucket length type
 * @param interval Time interval
 * @return Value in bucket length type
 */
unsigned long interval_cast(netpp::TimerInterval interval)
{
	return static_cast<unsigned long>(interval);
}

/**
 * @brief Cast wheel's bucket length type into time interval
 * @param size bucket length
 * @return Value in time interval
 */
netpp::TimerInterval bucket_size_cast(unsigned long size)
{
	return static_cast<netpp::TimerInterval>(size);
}
}

namespace netpp {
TimeWheel::TimeWheel(EventLoop *loop)
	: m_tickTimer(loop), m_timeOutBucketIndex(0)
{
	if (!m_buckets.empty())
	{
		m_tickTimer.setOnTimeout(std::bind(&TimeWheel::tick, this));
		m_tickTimer.setInterval(1000);
		m_tickTimer.setSingleShot(false);
		m_tickTimer.start();
	}
}

void TimeWheel::addToWheel(const std::shared_ptr<WheelEntry> &entry)
{
	if (!m_buckets.empty())
	{
		// find position for this entry
		if (::interval_cast(entry->timeoutTick) >= m_buckets.size())
			entry->timeoutTick = ::bucket_size_cast(m_buckets.size()) - 1;
		TimerInterval currentPos = (m_timeOutBucketIndex == 0) ? (!m_buckets.empty() - 1) : (m_timeOutBucketIndex - 1);
		entry->wheelIndex = (currentPos + entry->timeoutTick) % ::bucket_size_cast(m_buckets.size());
		entry->expire = false;
		m_buckets[::interval_cast(entry->wheelIndex)].insert(entry);
	}
}

void TimeWheel::removeFromWheel(const std::shared_ptr<WheelEntry> &entry)
{
	TimerInterval index = entry->wheelIndex;
	if (::interval_cast(entry->wheelIndex) >= m_buckets.size())
		return;
	auto &bucket = m_buckets[::interval_cast(index)];
	auto entryIt = bucket.find(entry);
	if (entryIt != bucket.end())
	{
		// if this entry in timeout bucket, just mark it, erase it in tick
		if (entry->wheelIndex == m_timeOutBucketIndex)
			entry->expire = true;
		else	// remove directly
			bucket.erase(entryIt);
	}
}

void TimeWheel::renew(const std::shared_ptr<WheelEntry> &entry)
{
	if (!m_buckets.empty())
	{
		if (::interval_cast(entry->timeoutTick) >= m_buckets.size())
			entry->timeoutTick = ::bucket_size_cast(m_buckets.size()) - 1;
		TimerInterval currentPos = (m_timeOutBucketIndex == 0) ? (!m_buckets.empty() - 1) : (m_timeOutBucketIndex - 1);
		TimerInterval previousIndex = entry->wheelIndex;
		entry->wheelIndex = (currentPos + entry->timeoutTick) % ::bucket_size_cast(m_buckets.size());
		entry->expire = false;

		if (::interval_cast(previousIndex) < m_buckets.size())
		{
			// remove entry from previous bucket
			auto &bucket = m_buckets[::interval_cast(previousIndex)];
			auto entryIt = bucket.find(entry);
			bucket.erase(entryIt);

			// add entry to target bucket
			m_buckets[::interval_cast(entry->wheelIndex)].insert(entry);
		}
	}
}

void TimeWheel::tick()
{
	++m_timeOutBucketIndex;
	if (m_timeOutBucketIndex == m_buckets.size())
		m_timeOutBucketIndex = 0;
	auto &bucket = m_buckets[m_timeOutBucketIndex];
	for (auto it = bucket.begin(); it != bucket.end();)
	{
		// make a copy in case onTimeout will erase itself
		std::shared_ptr<WheelEntry> entry = (*it);
		LOG_INFO("wheel entry time out");
		if (!entry->expire)
		{
			if (entry->callback)
				entry->callback();
		}
		if (entry->expire || entry->singleShot)
			it = bucket.erase(it);
		else
			++it;
	}
}
}
