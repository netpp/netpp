#ifndef NETPP_TIME_WHEEL_H
#define NETPP_TIME_WHEEL_H

#include <unordered_set>
#include <memory>
#include "Timer.h"
#include "support/Types.h"
#include <string>
#include <limits>
#include "TimerData.h"

namespace netpp {
class TimeWheel {
public:
	struct WheelEntryData : public TimerData {
	public:
		WheelEntryData()
		: TimerData{true, 1000},
		  expire{false}, timerId{nullptr}, wheelIndex{0}
		{}
		~WheelEntryData() override = default;

		bool expire;
		void *timerId;
		TimerInterval wheelIndex;
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

	Timer m_tickTimer;
	unsigned m_timeOutBucketIndex;

	std::array<std::unordered_map<void *, WheelEntryData>, 600> m_buckets;
};
}

#endif
