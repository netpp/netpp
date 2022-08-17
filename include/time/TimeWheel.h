#ifndef NETPP_TIME_WHEEL_H
#define NETPP_TIME_WHEEL_H

#include <unordered_set>
#include <memory>
#include "Timer.h"
#include "support/Types.h"
#include <string>
#include <limits>
#include "TimerData.h"
#include <functional>

namespace netpp {
/**
 * @brief Support up to 2,073,600 seconds(24 days)
 */
class TimeWheel {
private:
	struct WheelIndexer {
		int dayWheel = 0;
		int hourWheel = 0;
		int minuteWheel = 0;
		int secondWheel = 0;
		bool operator==(const WheelIndexer &other) const
		{
			return dayWheel == other.dayWheel
			&& hourWheel == other.hourWheel
			&& minuteWheel == other.minuteWheel
			&& secondWheel == other.secondWheel;
		}
	};

public:
	struct WheelEntryData : public TimerData {
	public:
		WheelEntryData()
		: TimerData{true, 1000},
		  expire{false}, timerId{nullptr}, wheelIndexer{}
		{}
		~WheelEntryData() override = default;

		bool expire;
		void *timerId;
		WheelIndexer wheelIndexer;
	};

	/**
	 * @brief Construct a TimeWheel
	 *
	 * @param loop			The event loop live in
	 */
	explicit TimeWheel(EventLoop *loop);

	void addToWheel(void *timerId, const WheelEntryData &data);
	void removeFromWheel(const WheelEntryData &data);
	void renew(const WheelEntryData &data);

private:
	void tick();

	WheelIndexer getIndexer(TimerInterval interval) const;

	Timer m_tickTimer;

	struct WheelEntryDataHash {
		size_t operator()(const WheelEntryData &data) const
		{
			return std::hash<void *>{}(data.timerId);
		}
	};

	struct WheelEntryDataEq {
		bool operator()(const WheelEntryData &lhs, const WheelEntryData &rhs) const
		{
			return lhs.timerId == rhs.timerId;
		}
	};

	using WheelEntryContainer = std::unordered_set<WheelEntryData, WheelEntryDataHash, WheelEntryDataEq>;
	static constexpr int daysMax = 24;
	static constexpr int hoursMax = 24;
	static constexpr int minutesMax = 60;
	static constexpr int secondsMax = 60;
	WheelEntryContainer m_dayWheel[daysMax];
	WheelEntryContainer m_hourWheel[hoursMax];
	WheelEntryContainer m_minWheel[minutesMax];
	WheelEntryContainer m_secWheel[secondsMax];
	WheelIndexer m_currentTick;
};
}

#endif
