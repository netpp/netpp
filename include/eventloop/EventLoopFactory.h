//
// Created by gaojian on 22-5-4.
//

#ifndef NETPP_EVENTLOOPFACTORY_H
#define NETPP_EVENTLOOPFACTORY_H

#include <memory>

namespace netpp {
class EventLoop;
class EventLoopFactory {
public:
	static std::unique_ptr<EventLoop> makeEventLoop(bool enableRunInLoop, bool enableTimeWheel);
};
}

#endif //NETPP_EVENTLOOPFACTORY_H
