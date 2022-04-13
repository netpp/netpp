//
// Created by gaojian on 2022/4/12.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
extern "C" {
#include <bits/timerfd.h>
}
#define private public
#define protected public
#include "eventloop/EventLoop.h"
#include "internal/handlers/TimerHandler.h"
#include "../MockSysCallEnvironment.h"

#undef private
#undef protected

using namespace netpp::internal::handlers;

class MockTimer : public SysCall {
public:
	MOCK_METHOD(int, mock_timerfd_create, (int, int), (override));
	MOCK_METHOD(int, mock_timerfd_settime, (int, int, const struct itimerspec *, struct itimerspec *), (override));
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));
	MOCK_METHOD(int, mock_close, (int), (override));
	MOCK_METHOD(::ssize_t, mock_read, (int, void *, ::size_t), (override));
};

class TimerHandlerTest : public testing::Test {
protected:
	void SetUp() override
	{
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		MockSysCallEnvironment::registerMock(&mock);
	}

	void TearDown() override
	{
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		MockSysCallEnvironment::restoreSysCall();
	}
	MockTimer mock;
};

TEST_F(TimerHandlerTest, CreateTimerTest)
{
	netpp::eventloop::EventLoop loop;
	EXPECT_CALL(mock, mock_timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(1)));
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, 1, EpollEventEq(EPOLLIN)))
		.Times(1);
	auto timerHandler = TimerHandler::makeTimerHandler(&loop);
	EXPECT_EQ(timerHandler->m_timerFd, 1);
}
