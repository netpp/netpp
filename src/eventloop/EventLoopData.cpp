//
// Created by gaojian on 2022/4/6.
//

#include "eventloop/EventLoopData.h"
#include "eventloop/EventLoop.h"

namespace netpp {
EventLoopData::EventLoopData()
		: runInLoopHandler{nullptr}, wheel{nullptr}
{}

EventLoopData::~EventLoopData() noexcept = default;
}
