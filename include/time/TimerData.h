//
// Created by gaojian on 22-8-14.
//

#ifndef NETPP_TIMERDATA_H
#define NETPP_TIMERDATA_H

#include <utility>

#include "support/Types.h"

namespace netpp {
struct TimerData {
public:
	TimerData(bool singleShot, TimerInterval interval, std::function<void()> callback = std::function<void()>())
		: singleShot{singleShot}, interval{interval}, callback{std::move(callback)}
	{}
	virtual ~TimerData() = default;

	bool singleShot;
	TimerInterval interval;
	std::function<void()> callback;
};
}

#endif //NETPP_TIMERDATA_H
