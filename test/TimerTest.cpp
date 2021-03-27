#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define private public
#define protected public
#include "internal/time/TimeWheel.h"
#include "EventLoop.h"
#include "MockSysCallEnvironment.h"
#include "internal/epoll/EpollEvent.h"
#include "internal/handlers/RunInLoopHandler.h"
#undef private
#undef protected

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
protected:
	void SetUp() override
	{
		timerTriggerCount = 0;
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
	}
	void TearDown() override
	{
		wheelTriggerCount = 0;
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		MockSysCallEnvironment::restoreSysCall();
	}

	MockTimer mock;
};

int TimerTest::timerTriggerCount = 0;
int TimerTest::wheelTriggerCount = 0;

/**
 * @brief ::itimerspec in repeatedly mode,
 * test if first trigger time within some range, and interval equal to msec
 * @note This test is NOT stable, maybe failed sometime
 */
MATCHER_P(RepeatedlyTimerEQ, msec, 
	std::string("equal to " + std::to_string(msec / 1000) + ":" + std::to_string(msec % 1000)))
{
	bool secInRange;
	// bool msecInRange;
	if (msec == 0)
	{
		secInRange = (arg->it_value.tv_sec == 0);
		// msecInRange = (arg->it_value.tv_nsec == 0);
	}
	else
	{
		::timespec now;
		::clock_gettime(CLOCK_MONOTONIC, &now);
		::time_t sec = now.tv_sec + msec / 1000;
		// long nsec = now.tv_nsec + (static_cast<long>(msec) % 1000) * 1000 * 1000;
		secInRange = ((sec - arg->it_value.tv_sec) < 1000);
		// msecInRange = ((nsec - arg->it_value.tv_nsec) < 1000000);
	}
	bool secEq;
	bool msecEq;
	if (msec == 0)
	{
		secEq = (arg->it_interval.tv_sec == 0);
		msecEq = (arg->it_interval.tv_nsec == 0);
	}
	else
	{
		secEq = (arg->it_interval.tv_sec == msec / 1000);
		msecEq = (arg->it_interval.tv_nsec / 1000000 == msec % 1000);
	}
	return secInRange && secEq && msecEq;
}

/**
 * @brief ::itimerspec in single shot mode,
 * test if first trigger time within some range, and interval equal to msec
 * @note This test is NOT stable, maybe failed sometime
 */
MATCHER_P(SingleShotTimerEQ, msec, "")
{
	bool secInRange;
	// bool msecInRange;
	if (msec == 0)
	{
		secInRange = (arg->it_value.tv_sec == 0);
		// msecInRange = (arg->it_value.tv_nsec == 0);
	}
	else
	{
		::timespec now;
		::clock_gettime(CLOCK_MONOTONIC, &now);
		::time_t sec = now.tv_sec + msec / 1000;
		// long nsec = now.tv_nsec + (static_cast<long>(msec) % 1000) * 1000 * 1000;
		secInRange = ((sec - arg->it_value.tv_sec) < 1000);
		// msecInRange = ((nsec - arg->it_value.tv_nsec) < 1000000);
	}
	bool secEq = (arg->it_interval.tv_sec == 0);
	bool msecEq = (arg->it_interval.tv_nsec == 0);
	return secInRange && secEq && msecEq;
}

TEST_F(TimerTest, SingleShotTimerTest)
{
	MockSysCallEnvironment::registerMock(&mock);

	netpp::EventLoop eventLoop;
	EXPECT_CALL(mock, mock_timerfd_create).Times(1);
	std::shared_ptr<netpp::time::Timer> timer = std::make_shared<netpp::time::Timer>(&eventLoop);
	// set interval without start do not apply changes
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->setInterval(1100);

	// not started timer can not stop
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->stop();

	// single shot, interval is 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, SingleShotTimerEQ(1100), testing::_))
		.Times(1);
	timer->start();
	// single shot, interval is 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, SingleShotTimerEQ(1200), testing::_))
		.Times(1);
	timer->setInterval(1200);

	// running timer can not start again
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->start();

	// set to 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, SingleShotTimerEQ(0), testing::_))
		.Times(1);
	timer->stop();
}

TEST_F(TimerTest, RepeatedlyTimerTest)
{
	MockSysCallEnvironment::registerMock(&mock);

	netpp::EventLoop eventLoop;
	EXPECT_CALL(mock, mock_timerfd_create).Times(1);
	std::shared_ptr<netpp::time::Timer> timer = std::make_shared<netpp::time::Timer>(&eventLoop);
	// set interval without start do not apply changes
	timer->setSingleShot(false);
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->setInterval(1100);

	// not started timer can not stop
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->stop();

	// single shot, interval is 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, RepeatedlyTimerEQ(1100), testing::_))
		.Times(1);
	timer->start();
	// single shot, interval is 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, RepeatedlyTimerEQ(1200), testing::_))
		.Times(1);
	timer->setInterval(1200);

	// running timer can not start again
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->start();

	// set to 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, RepeatedlyTimerEQ(0), testing::_))
		.Times(1);
	timer->stop();
}

TEST_F(TimerTest, SetSingleShotWhileRunningTest)
{
	MockSysCallEnvironment::registerMock(&mock);

	netpp::EventLoop eventLoop;
	EXPECT_CALL(mock, mock_timerfd_create).Times(1);
	std::shared_ptr<netpp::time::Timer> timer = std::make_shared<netpp::time::Timer>(&eventLoop);
	// set interval without start do not apply changes
	EXPECT_CALL(mock, mock_timerfd_settime).Times(0);
	timer->setSingleShot(true);
	timer->setInterval(1100);

	// single shot, interval is 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, SingleShotTimerEQ(1100), testing::_))
		.Times(1);
	timer->start();
	// single shot, interval is 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, RepeatedlyTimerEQ(1100), testing::_))
		.Times(1);
	timer->setSingleShot(false);
	// single shot, interval is 0
	EXPECT_CALL(mock, mock_timerfd_settime(testing::_, testing::_, SingleShotTimerEQ(1100), testing::_))
		.Times(1);
	timer->setSingleShot(true);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
TEST_F(TimerTest, TimerTriggerTest)
{
	MockSysCallEnvironment::registerMock(&mock);

	netpp::EventLoop eventLoop;
	EXPECT_CALL(mock, mock_timerfd_create).Times(1);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, GetPtrFromEpollCtl()))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(0)));
	std::shared_ptr<netpp::time::Timer> timer = std::make_shared<netpp::time::Timer>(&eventLoop);
	timer->setOnTimeout([&]{ ++timerTriggerCount; });

	auto *timerHandler = static_cast<netpp::internal::epoll::EpollEvent *>(MockSysCallEnvironment::ptrFromEpollCtl);
	ASSERT_NE(timerHandler, nullptr);
	netpp::internal::epoll::Epoll *epoll = eventLoop.getPoll();
	::epoll_event ev[1];
	ev[0].data.ptr = static_cast<void *>(timerHandler);
	EXPECT_CALL(mock, mock_epoll_wait(testing::_, testing::_, testing::_, testing::_))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLIN), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)));
	epoll->poll();

	EXPECT_CALL(mock, mock_read).WillOnce(testing::Return(1));
	timerHandler->handleEvents();
	EXPECT_EQ(timerTriggerCount, 1);

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}
#pragma GCC diagnostic pop

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
