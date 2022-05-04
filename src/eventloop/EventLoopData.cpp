//
// Created by gaojian on 2022/4/6.
//

#include "eventloop/EventLoopData.h"
#include "eventloop/EventLoop.h"

namespace netpp::eventloop {
EventLoopData::EventLoopData()
		: runInLoopHandler{nullptr}, wheel{nullptr}, signalHandler{nullptr}
{}

EventLoopData::~EventLoopData() noexcept = default;
}
