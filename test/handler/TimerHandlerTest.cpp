//
// Created by gaojian on 2022/4/12.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <cstring>
extern "C" {
#include <sys/timerfd.h>
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
	MOCK_METHOD(int, mock_clock_gettime, (clockid_t __clock_id, struct timespec *__tp), (override));
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
	int fakeTimerFd = 1;
	netpp::eventloop::EventLoop loop;
	EXPECT_CALL(mock, mock_timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(fakeTimerFd)));
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, fakeTimerFd, EpollEventEq(EPOLLIN)))
		.Times(1);
	auto timerHandler = TimerHandler::makeTimerHandler(&loop);
	EXPECT_EQ(timerHandler->m_timerFd, fakeTimerFd);

	EXPECT_CALL(mock, mock_close(fakeTimerFd));
}

TEST_F(TimerHandlerTest, HandleTimeoutTest)
{
	int fakeTimerFd = 1;
	netpp::eventloop::EventLoop loop;
	EXPECT_CALL(mock, mock_timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
			.Times(1)
			.WillOnce(testing::DoAll(testing::Return(fakeTimerFd)));
	TimerHandler handler(&loop);
	bool didRunCallback = false;
	handler.m_timeoutCallback = [&didRunCallback]{ didRunCallback = true; };
	EXPECT_CALL(mock, mock_read(fakeTimerFd, testing::_, testing::_))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(1)));
	handler.handleIn();
	EXPECT_TRUE(didRunCallback);
}

TEST_F(TimerHandlerTest, HandleWrongCallbackTest)
{
	netpp::eventloop::EventLoop loop;
	TimerHandler handler(&loop);
	handler.m_timeoutCallback = std::function<void()>();
	// expect not to crash
	handler.handleIn();
}

TEST_F(TimerHandlerTest, StopTimerTest)
{
	netpp::eventloop::EventLoop loop;
	TimerHandler handler(&loop);

	::itimerspec timerSpec{};
	std::memset(&timerSpec, 0, sizeof(::itimerspec));
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, TFD_TIMER_ABSTIME, TimerspecEq(timerSpec), nullptr))
		.Times(1);
	handler.stopTimer();
}

TEST_F(TimerHandlerTest, RunSignleShotTimerTest)
{
	netpp::eventloop::EventLoop loop;
	TimerHandler handler(&loop);

	{
		::timespec now{};
		std::memset(&now, 0, sizeof(::timespec));
		::itimerspec timerSpec{};
		std::memset(&timerSpec, 0, sizeof(::itimerspec));
		timerSpec.it_value.tv_sec = 1;
		timerSpec.it_value.tv_nsec = 0;
		timerSpec.it_interval.tv_sec = 0;
		timerSpec.it_interval.tv_nsec = 0;
		EXPECT_CALL(mock, mock_clock_gettime(CLOCK_MONOTONIC, testing::_))
				.Times(1)
				.WillOnce(testing::DoAll(testing::SetArgPointee<1>(now)));
		EXPECT_CALL(mock, mock_timerfd_settime(testing::_, TFD_TIMER_ABSTIME, TimerspecEq(timerSpec), nullptr))
				.Times(1);
		handler.setIntervalAndRun(1000, false, [] {});
	}
	{
		::timespec now{};
		std::memset(&now, 0, sizeof(::timespec));
		now.tv_sec = 100;
		now.tv_nsec = 1000;
		::itimerspec timerSpec{};
		std::memset(&timerSpec, 0, sizeof(::itimerspec));
		timerSpec.it_value.tv_sec = 101;
		timerSpec.it_value.tv_nsec = 1000;
		timerSpec.it_interval.tv_sec = 0;
		timerSpec.it_interval.tv_nsec = 0;
		EXPECT_CALL(mock, mock_clock_gettime(CLOCK_MONOTONIC, testing::_))
				.Times(1)
				.WillOnce(testing::DoAll(testing::SetArgPointee<1>(now)));
		EXPECT_CALL(mock, mock_timerfd_settime(testing::_, TFD_TIMER_ABSTIME, TimerspecEq(timerSpec), nullptr))
				.Times(1);
		handler.setIntervalAndRun(1000, false, [] {});
	}
	{
		::timespec now{};
		std::memset(&now, 0, sizeof(::timespec));
		::itimerspec timerSpec{};
		std::memset(&timerSpec, 0, sizeof(::itimerspec));
		timerSpec.it_value.tv_sec = 1;
		timerSpec.it_value.tv_nsec = 1000000;
		timerSpec.it_interval.tv_sec = 0;
		timerSpec.it_interval.tv_nsec = 0;
		EXPECT_CALL(mock, mock_clock_gettime(CLOCK_MONOTONIC, testing::_))
				.Times(1)
				.WillOnce(testing::DoAll(testing::SetArgPointee<1>(now)));
		EXPECT_CALL(mock, mock_timerfd_settime(testing::_, TFD_TIMER_ABSTIME, TimerspecEq(timerSpec), nullptr))
				.Times(1);
		handler.setIntervalAndRun(1001, false, [] {});
	}
	{
		::timespec now{};
		std::memset(&now, 0, sizeof(::timespec));
		::itimerspec timerSpec{};
		std::memset(&timerSpec, 0, sizeof(::itimerspec));
		timerSpec.it_value.tv_sec = 10;
		timerSpec.it_value.tv_nsec = 10000000;
		timerSpec.it_interval.tv_sec = 0;
		timerSpec.it_interval.tv_nsec = 0;
		EXPECT_CALL(mock, mock_clock_gettime(CLOCK_MONOTONIC, testing::_))
				.Times(1)
				.WillOnce(testing::DoAll(testing::SetArgPointee<1>(now)));
		EXPECT_CALL(mock, mock_timerfd_settime(testing::_, TFD_TIMER_ABSTIME, TimerspecEq(timerSpec), nullptr))
				.Times(1);
		handler.setIntervalAndRun(10010, false, [] {});
	}
}

TEST_F(TimerHandlerTest, RunRepeatlyTimerTest)
{
	netpp::eventloop::EventLoop loop;
	TimerHandler handler(&loop);

	{
		::timespec now{};
		std::memset(&now, 0, sizeof(::timespec));
		::itimerspec timerSpec{};
		std::memset(&timerSpec, 0, sizeof(::itimerspec));
		timerSpec.it_value.tv_sec = 1;
		timerSpec.it_value.tv_nsec = 0;
		timerSpec.it_interval.tv_sec = 1;
		timerSpec.it_interval.tv_nsec = 0;
		EXPECT_CALL(mock, mock_clock_gettime(CLOCK_MONOTONIC, testing::_))
				.Times(1)
				.WillOnce(testing::DoAll(testing::SetArgPointee<1>(now)));
		EXPECT_CALL(mock, mock_timerfd_settime(testing::_, TFD_TIMER_ABSTIME, TimerspecEq(timerSpec), nullptr))
				.Times(1);
		handler.setIntervalAndRun(1000, true, [] {});
	}
	{
		::timespec now{};
		std::memset(&now, 0, sizeof(::timespec));
		now.tv_sec = 100;
		now.tv_nsec = 1000;
		::itimerspec timerSpec{};
		std::memset(&timerSpec, 0, sizeof(::itimerspec));
		timerSpec.it_value.tv_sec = 101;
		timerSpec.it_value.tv_nsec = 1000;
		timerSpec.it_interval.tv_sec = 101;
		timerSpec.it_interval.tv_nsec = 1000;
		EXPECT_CALL(mock, mock_clock_gettime(CLOCK_MONOTONIC, testing::_))
				.Times(1)
				.WillOnce(testing::DoAll(testing::SetArgPointee<1>(now)));
		EXPECT_CALL(mock, mock_timerfd_settime(testing::_, TFD_TIMER_ABSTIME, TimerspecEq(timerSpec), nullptr))
				.Times(1);
		handler.setIntervalAndRun(1000, true, [] {});
	}
	{
		::timespec now{};
		std::memset(&now, 0, sizeof(::timespec));
		::itimerspec timerSpec{};
		std::memset(&timerSpec, 0, sizeof(::itimerspec));
		timerSpec.it_value.tv_sec = 1;
		timerSpec.it_value.tv_nsec = 1000000;
		timerSpec.it_interval.tv_sec = 1;
		timerSpec.it_interval.tv_nsec = 1000000;
		EXPECT_CALL(mock, mock_clock_gettime(CLOCK_MONOTONIC, testing::_))
				.Times(1)
				.WillOnce(testing::DoAll(testing::SetArgPointee<1>(now)));
		EXPECT_CALL(mock, mock_timerfd_settime(testing::_, TFD_TIMER_ABSTIME, TimerspecEq(timerSpec), nullptr))
				.Times(1);
		handler.setIntervalAndRun(1001, true, [] {});
	}
	{
		::timespec now{};
		std::memset(&now, 0, sizeof(::timespec));
		::itimerspec timerSpec{};
		std::memset(&timerSpec, 0, sizeof(::itimerspec));
		timerSpec.it_value.tv_sec = 10;
		timerSpec.it_value.tv_nsec = 10000000;
		timerSpec.it_interval.tv_sec = 10;
		timerSpec.it_interval.tv_nsec = 10000000;
		EXPECT_CALL(mock, mock_clock_gettime(CLOCK_MONOTONIC, testing::_))
				.Times(1)
				.WillOnce(testing::DoAll(testing::SetArgPointee<1>(now)));
		EXPECT_CALL(mock, mock_timerfd_settime(testing::_, TFD_TIMER_ABSTIME, TimerspecEq(timerSpec), nullptr))
				.Times(1);
		handler.setIntervalAndRun(10010, true, [] {});
	}
}
