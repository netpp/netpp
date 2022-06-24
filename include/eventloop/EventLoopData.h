//
// Created by gaojian on 2022/4/6.
//

#ifndef NETPP_EVENTLOOPDATA_H
#define NETPP_EVENTLOOPDATA_H

#include <memory>

namespace netpp {
class RunInLoopHandler;
class SignalHandler;
class TimeWheel;
struct EventLoopData {
	EventLoopData();

	virtual ~EventLoopData() noexcept;

	std::shared_ptr<RunInLoopHandler> runInLoopHandler;

	std::shared_ptr<TimeWheel> wheel;
	std::shared_ptr<SignalHandler> signalHandler;
};
}

#endif //NETPP_EVENTLOOPDATA_H
