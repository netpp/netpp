#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define private public
#define protected public
#include "time/TimeWheel.h"
#include "eventloop/EventLoop.h"
#include "internal/epoll/EpollEvent.h"
#include "internal/handlers/RunInLoopHandler.h"
#include "internal/epoll/Epoll.h"
#include "Application.h"

#undef private
#undef protected

class TimerTest : public testing::Test {
public:
protected:
	void SetUp() override {}
	void TearDown() override {}
};

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

TEST_F(TimerTest, SetTimerPropertyTest)
{
	netpp::Application app;
	netpp::time::Timer timer(app.loopManager()->mainLoop());
	EXPECT_EQ(timer.interval(), 1000);
	EXPECT_EQ(timer.singleShot(), true);
	EXPECT_EQ(timer.running(), false);

	timer.setInterval(2000);
	EXPECT_EQ(timer.interval(), 2000);

	timer.setSingleShot(false);
	EXPECT_EQ(timer.singleShot(), false);
}

TEST_F(TimerTest, SingleShotTimerTest)
{
	int timerTriggerCount = 0;
	try {
		netpp::eventloop::EventLoop eventLoop;
		netpp::time::Timer timer(&eventLoop);
		netpp::time::Timer quitLoopTimer(&eventLoop);

		timer.setInterval(10);
		timer.setSingleShot(true);
		timer.setOnTimeout([&timerTriggerCount]{ ++timerTriggerCount; });
		quitLoopTimer.setInterval(50);
		quitLoopTimer.setSingleShot(true);
		quitLoopTimer.setOnTimeout([]{ throw std::runtime_error("quit"); });
		timer.start();
		quitLoopTimer.start();
		eventLoop.run();
	} catch (const std::runtime_error &e) {
	}
	EXPECT_EQ(timerTriggerCount, 1);
}

TEST_F(TimerTest, RepeatedlyTimerTest)
{
	int timerTriggerCount = 0;
	try {
		netpp::eventloop::EventLoop eventLoop;
		netpp::time::Timer timer(&eventLoop);

		timer.setInterval(10);
		timer.setSingleShot(false);
		timer.setOnTimeout([&timerTriggerCount]{
			++timerTriggerCount;
			if (timerTriggerCount > 5)
				throw std::runtime_error("quit");
		});
		timer.start();
		eventLoop.run();
	} catch (const std::runtime_error &e) {
	}
	EXPECT_GT(timerTriggerCount, 5);
}

TEST_F(TimerTest, SetSingleShotWhileRunningTest)
{
	int timerTriggerCount = 0;
	try {
		netpp::eventloop::EventLoop eventLoop;
		netpp::time::Timer timer(&eventLoop);
		netpp::time::Timer quitLoopTimer(&eventLoop);

		timer.setInterval(10);
		timer.setSingleShot(false);
		timer.setOnTimeout([&timerTriggerCount, &timer]{ ++timerTriggerCount; timer.setSingleShot(true); });
		quitLoopTimer.setInterval(50);
		quitLoopTimer.setSingleShot(true);
		quitLoopTimer.setOnTimeout([]{ throw std::runtime_error("quit"); });
		timer.start();
		quitLoopTimer.start();
		eventLoop.run();
	} catch (const std::runtime_error &e) {
	}
	EXPECT_EQ(timerTriggerCount, 2);
}
