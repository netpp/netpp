//
// Created by gaojian on 22-5-4.
//

#include "eventloop/EventLoopFactory.h"
#include "eventloop/EventLoop.h"
#include "eventloop/EventLoopData.h"
#include "epoll/handlers/RunInLoopHandler.h"
#include "time/TimeWheel.h"

namespace netpp {
std::unique_ptr<EventLoop> EventLoopFactory::makeEventLoop(bool enableRunInLoop, bool enableTimeWheel)
{
	auto loop = std::make_unique<EventLoop>();
	loop->m_loopData = std::make_unique<EventLoopData>();
	if (enableRunInLoop)
		loop->m_loopData->runInLoopHandler = std::make_shared<RunInLoopHandler>(loop.get());
	if (enableTimeWheel)
		loop->m_loopData->wheel = std::make_shared<netpp::TimeWheel>(loop.get());
	return loop;
}
}
