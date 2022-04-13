//
// Created by gaojian on 2022/4/6.
//

#include "eventloop/EventLoopData.h"
#include "eventloop/EventLoop.h"

namespace netpp::eventloop {
EventLoopData::EventLoopData()
		: eventLoop{std::make_unique<EventLoop>()}, runInLoopHandler{nullptr}
{}

EventLoopData::~EventLoopData() noexcept = default;

MainEventLoopData::MainEventLoopData()
		: EventLoopData(), wheel{nullptr}, signalHandler{nullptr}
{}

MainEventLoopData::~MainEventLoopData() noexcept = default;
}
