//
// Created by gaojian on 2022/4/6.
//

#ifndef NETPP_EVENTLOOPDATA_H
#define NETPP_EVENTLOOPDATA_H

#include <memory>

namespace netpp {
namespace internal::handlers {
class RunInLoopHandler;
class SignalHandler;
}
namespace time {
class TimeWheel;
}
}

namespace netpp::eventloop {
class EventLoop;
struct EventLoopData {
	EventLoopData();

	virtual ~EventLoopData() noexcept;

	std::unique_ptr<EventLoop> eventLoop;
	std::shared_ptr<netpp::internal::handlers::RunInLoopHandler> runInLoopHandler;
};

struct MainEventLoopData : public EventLoopData {
	MainEventLoopData();

	~MainEventLoopData() noexcept override;

	std::shared_ptr<netpp::time::TimeWheel> wheel;
	std::shared_ptr<netpp::internal::handlers::SignalHandler> signalHandler;
};
}

#endif //NETPP_EVENTLOOPDATA_H
