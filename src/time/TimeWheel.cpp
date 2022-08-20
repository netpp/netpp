#include "time/TimeWheel.h"
#include "eventloop/EventLoop.h"

namespace netpp {
TimeWheel::TimeWheel(EventLoop *loop)
	: m_tickTimer(loop)
{
	m_tickTimer.setOnTimeout(std::bind(&TimeWheel::tick, this));
	m_tickTimer.setInterval(1000);
	m_tickTimer.setSingleShot(false);
	m_tickTimer.start();
}

void TimeWheel::addToWheel(void *timerId, const std::shared_ptr<WheelEntryData>& data)
{
	data->expire = false;
	auto indexer = getIndexer(data->interval);
	data->wheelIndexer = indexer;
	data->timerId = timerId;
	if (indexer.dayWheel != m_currentTick.dayWheel)
		m_dayWheel[indexer.dayWheel].emplace(data);
	else if (indexer.hourWheel != m_currentTick.hourWheel)
		m_hourWheel[indexer.hourWheel].emplace(data);
	else if (indexer.minuteWheel != m_currentTick.minuteWheel)
		m_minWheel[indexer.minuteWheel].emplace(data);
	else
		m_secWheel[indexer.secondWheel].emplace(data);
}

void TimeWheel::removeFromWheel(const std::shared_ptr<WheelEntryData>& data)
{
	WheelEntryContainer *targetContainer = nullptr;
	if (data->wheelIndexer.dayWheel == m_currentTick.dayWheel)
	{
		if (data->wheelIndexer.hourWheel == m_currentTick.hourWheel)
		{
			if (data->wheelIndexer.minuteWheel == m_currentTick.minuteWheel)
				targetContainer = m_secWheel + data->wheelIndexer.secondWheel;
			else
				targetContainer = m_minWheel + data->wheelIndexer.minuteWheel;
		}
		else
		{
			targetContainer = m_hourWheel + data->wheelIndexer.hourWheel;
		}
	}
	else
	{
		targetContainer = m_dayWheel + data->wheelIndexer.dayWheel;
	}

	if (targetContainer)
	{
		auto it = targetContainer->find(data);
		if (it != targetContainer->end())
		{
			auto &entry = *it;
			if (entry->wheelIndexer.secondWheel == m_currentTick.secondWheel)
				entry->expire = true;	// field expire is not for hash
			else
				targetContainer->erase(it);
		}
	}
}

void TimeWheel::renew(const std::shared_ptr<WheelEntryData>& data)
{
	removeFromWheel(data);
	addToWheel(data->timerId, data);
}

void TimeWheel::tick()
{
	bool dayChanged = false;
	bool hourChanged = false;
	bool minuteChanged = false;

	// update indexer
	++m_currentTick.secondWheel;
	if (m_currentTick.secondWheel >= secondsMax)
	{
		m_currentTick.secondWheel = 0;
		++m_currentTick.minuteWheel;
		minuteChanged = true;
	}
	if (m_currentTick.minuteWheel >= minutesMax)
	{
		m_currentTick.minuteWheel = 0;
		++m_currentTick.hourWheel;
		hourChanged = true;
	}
	if (m_currentTick.hourWheel >= hoursMax)
	{
		m_currentTick.hourWheel = 0;
		++m_currentTick.dayWheel;
		dayChanged = true;
	}
	if (m_currentTick.dayWheel >= daysMax)
	{
		m_currentTick.dayWheel = 0;
	}

	if (dayChanged)
	{
		auto &entryInDay = m_dayWheel[m_currentTick.dayWheel];
		auto it = entryInDay.begin();
		while (it != entryInDay.end())
		{
			std::shared_ptr<WheelEntryData> data = *it;
			if (data)
				m_hourWheel[data->wheelIndexer.hourWheel].emplace(data);
			entryInDay.erase(it);
		}
	}
	if (hourChanged)
	{
		auto &entryInHour = m_hourWheel[m_currentTick.hourWheel];
		auto it = entryInHour.begin();
		while (it != entryInHour.end())
		{
			std::shared_ptr<WheelEntryData> data = *it;
			if (data)
				m_minWheel[data->wheelIndexer.minuteWheel].emplace(data);
			entryInHour.erase(it);
		}
	}
	if (minuteChanged)
	{
		auto &entryInMin = m_minWheel[m_currentTick.minuteWheel];
		auto it = entryInMin.begin();
		while (it != entryInMin.end())
		{
			std::shared_ptr<WheelEntryData> data = *it;
			if (data)
				m_secWheel[data->wheelIndexer.secondWheel].emplace(data);
			entryInMin.erase(it);
		}
	}

	auto &entryInSec = m_secWheel[m_currentTick.secondWheel];
	auto it = entryInSec.begin();
	while (it != entryInSec.end())
	{
		std::shared_ptr<WheelEntryData> data = *it;
		if (data)
		{
			if (!data->expire)
			{
				if (data->callback)
					data->callback();
			}
			// if not single shot, add entry to wheel
			if (!data->expire && !data->singleShot)
			{
				addToWheel(data->timerId, data);
			}
		}
		it = entryInSec.erase(it);
	}
}

TimeWheel::WheelIndexer TimeWheel::getIndexer(netpp::TimerInterval interval) const
{
	WheelIndexer indexer;
	// ceil
	if (interval == 0 || interval % 1000 != 0)
		interval += 1000;
	int intervalInSec = static_cast<int>(interval / 1000 % secondsMax);
	int intervalInMin = static_cast<int>((interval / 1000 / secondsMax) % minutesMax);
	int intervalInHour = static_cast<int>((interval / 1000 / secondsMax / minutesMax) % hoursMax);
	int intervalInDay = static_cast<int>(interval / 1000 / secondsMax / minutesMax / hoursMax);
	if (intervalInDay > daysMax)
		intervalInDay = daysMax;
	indexer.secondWheel = m_currentTick.secondWheel + intervalInSec;
	if (indexer.secondWheel >= secondsMax)
	{
		indexer.secondWheel -= secondsMax;
		++intervalInMin;
	}
	indexer.minuteWheel = m_currentTick.minuteWheel + intervalInMin;
	if (indexer.minuteWheel >= minutesMax)
	{
		indexer.minuteWheel -= minutesMax;
		++intervalInHour;
	}
	indexer.hourWheel = m_currentTick.hourWheel + intervalInHour;
	if (indexer.hourWheel >= hoursMax)
	{
		indexer.hourWheel -= hoursMax;
		++intervalInDay;
	}
	indexer.dayWheel = m_currentTick.dayWheel + intervalInDay;
	if (indexer.dayWheel >= daysMax)
	{
		indexer.dayWheel -= daysMax;
	}

	return indexer;
}
}
