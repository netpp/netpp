//
// Created by gaojian on 22-5-2.
//


#ifndef NETPP_MOCKEVENTLOOP_H
#define NETPP_MOCKEVENTLOOP_H

#include <gmock/gmock.h>
#include "eventloop/EventLoop.h"

class MockEventLoop : public netpp::eventloop::EventLoop {
public:
	MOCK_METHOD(void, run, ());
	MOCK_METHOD(void, addEventHandlerToLoop, (const EventLoop::Handler &));
	MOCK_METHOD(void, removeEventHandlerFromLoop, (const EventLoop::Handler &));
	MOCK_METHOD(netpp::internal::epoll::Epoll *, getPoll, ());
	MOCK_METHOD(EventLoop *, thisLoop, ());
	MOCK_METHOD(void, runInLoop, (std::function<void()>));
};

#endif //NETPP_MOCKEVENTLOOP_H
