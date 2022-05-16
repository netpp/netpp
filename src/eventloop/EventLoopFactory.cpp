//
// Created by gaojian on 22-5-4.
//

#include "eventloop/EventLoopFactory.h"
#include "eventloop/EventLoop.h"
#include "eventloop/EventLoopData.h"
#include "internal/handlers/RunInLoopHandler.h"
#include "internal/handlers/SignalHandler.h"
#include "Config.h"
#include "time/TimeWheel.h"

namespace netpp::eventloop {
std::unique_ptr<EventLoop> EventLoopFactory::makeEventLoop(bool enableRunInLoop, bool enableTimeWheel, bool enableHandleSignal,
														   const Config &config)
{
	auto loop = std::make_unique<EventLoop>();
	loop->m_loopData = std::make_unique<EventLoopData>();
	if (enableRunInLoop)
		loop->m_loopData->runInLoopHandler = netpp::internal::handlers::RunInLoopHandler::makeRunInLoopHandler(loop.get());
	if (enableTimeWheel)
		loop->m_loopData->wheel = std::make_unique<netpp::time::TimeWheel>(loop.get(), config.tickTimer.tickInterval, config.tickTimer.maxLength);
	if (enableHandleSignal)
		loop->m_loopData->signalHandler = netpp::internal::handlers::SignalHandler::makeSignalHandler(loop.get(), config.eventHandler, config.signal.interestedSignals);
	return loop;
}
}
