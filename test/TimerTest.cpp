#include <gtest/gtest.h>
#include "time/TimeWheel.h"
#include "EventLoop.h"
#include "support/ThreadPool.hpp"
#include "MockSysCallEnvironment.h"
#include <gmock/gmock.h>

class MockTimer : public SysCall {
public:
	MOCK_METHOD(int, mock_epoll_wait, (int, struct epoll_event *, int, int), (override));
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));

	MOCK_METHOD(::ssize_t, mock_read, (int, void *, ::size_t), (override));
	MOCK_METHOD(int, mock_timerfd_create, (int, int), (override));
	MOCK_METHOD(int, mock_timerfd_settime, (int, int, const struct itimerspec *, struct itimerspec *), (override));
};

class TimerTest : public testing::Test {
public:
	static int timerTriggerCount;
	static int wheelTriggerCount;
	static void *epollEventPtr;
protected:
	void SetUp() override { timerTriggerCount = 0; }
	void TearDown() override
	{
		wheelTriggerCount = 0;
		MockSysCallEnvironment::restoreSysCall();
	}

	MockTimer mock;
};

int TimerTest::timerTriggerCount = 0;
int TimerTest::wheelTriggerCount = 0;
void *TimerTest::epollEventPtr = nullptr;

MATCHER(GetTimerEpollEvent, "")
{
	if (arg)
	{
		TimerTest::epollEventPtr = arg->data.ptr;
		return arg->data.ptr;
	}
	return false;
}

MATCHER_P(TimerIntervalEQ, msec, 
	std::string("equal to " + std::to_string(msec / 1000) + ":" + std::to_string(msec % 1000)))
{
	bool secEq = (arg->it_interval.tv_sec == msec / 1000);
	bool msecEq = (arg->it_interval.tv_nsec / 1000000 == msec % 1000);
	return secEq && msecEq;
}

MATCHER_P(SingleShotTimerEQ, msec, "")
{
	return false;
}

TEST_F(TimerTest, SingleShotTimerTest)
{
	MockSysCallEnvironment::registerMock(&mock);

	EXPECT_CALL(mock, mock_epoll_ctl).Times(1);
	netpp::EventLoop eventLoop;
	EXPECT_CALL(mock, mock_timerfd_create).Times(1);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, GetTimerEpollEvent()))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(0)));
	std::shared_ptr<netpp::time::Timer> timer = std::make_shared<netpp::time::Timer>(&eventLoop);
	timer->setOnTimeout([]{});
	// set interval without start do not apply changes
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->setInterval(1100);

	// not started timer can not stop
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->stop();

	// single shot, interval is 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, TimerIntervalEQ(0), testing::_))
		.Times(1);
	timer->start();
	// single shot, interval is 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, TimerIntervalEQ(0), testing::_))
		.Times(1);
	timer->setInterval(1200);

	// running timer can not start again
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->start();

	// set to 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, TimerIntervalEQ(0), testing::_))
		.Times(1);
	timer->stop();

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, nullptr))
		.Times(1);
}

TEST_F(TimerTest, Timer)
{
	netpp::EventLoop loop;
	std::shared_ptr<netpp::time::Timer> timer;
	loop.runInLoop([&]{
		timer = std::make_shared<netpp::time::Timer>(&loop);
		timer->setInterval(10);
		timer->setSingleShot(false);
		timer->setOnTimeout([]{
			++timerTriggerCount;
			if (timerTriggerCount == 6)
				throw std::runtime_error("");
		});
		timer->start();
	});
	EXPECT_THROW(loop.run(), std::runtime_error);
	EXPECT_EQ(timerTriggerCount, 6);
}

class TestWheelEntry : public netpp::internal::time::TimeWheelEntry {
public:
	explicit TestWheelEntry(std::weak_ptr<netpp::internal::time::TimeWheel> wheel) : _wheel{wheel} {}
	~TestWheelEntry() override = default;
	void onTimeout() override;
private:
	std::weak_ptr<netpp::internal::time::TimeWheel> _wheel;
};

void TestWheelEntry::onTimeout()
{
	auto wheel = _wheel.lock();
	wheel->addToWheel(std::make_shared<TestWheelEntry>(wheel));
	auto removeWheel = std::make_shared<TestWheelEntry>(wheel);
	++TimerTest::wheelTriggerCount;
}

TEST_F(TimerTest, TimeWheel)
{
	netpp::EventLoop loop;
	std::shared_ptr<netpp::time::Timer> timer;
	std::shared_ptr<netpp::internal::time::TimeWheel> wheel;
	loop.runInLoop([&]{
		timer = std::make_shared<netpp::time::Timer>(&loop);
		timer->setInterval(83);
		timer->setSingleShot(true);
		timer->setOnTimeout([]{ throw std::runtime_error(""); });
		timer->start();
		wheel = std::make_shared<netpp::internal::time::TimeWheel>(&loop, 10, 2);
		wheel->addToWheel(std::make_shared<TestWheelEntry>(wheel));
	});
	EXPECT_THROW(loop.run(), std::runtime_error);
	EXPECT_LE(wheelTriggerCount, 10);
	EXPECT_GE(wheelTriggerCount, 6);
}
