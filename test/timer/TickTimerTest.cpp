//
// Created by gaojian on 2022/4/12.
//

#include <gtest/gtest.h>
#include "time/TimeWheel.h"
#include "time/TickTimer.h"
#include "time/Timer.h"
#include "eventloop/EventLoop.h"

class TickTimerTest : public testing::Test {
public:
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(TickTimerTest, SetTimerPropertyTest)
{
	netpp::eventloop::EventLoop eventLoop;
	auto wheel = std::make_shared<netpp::time::TimeWheel>(&eventLoop, 10, 10);
	netpp::time::TickTimer timer(wheel);

	EXPECT_EQ(timer.interval(), 1000);
	EXPECT_EQ(timer.singleShot(), true);
	EXPECT_EQ(timer.running(), false);

	timer.setInterval(2000);
	EXPECT_EQ(timer.interval(), 2000);

	timer.setSingleShot(false);
	EXPECT_EQ(timer.singleShot(), false);
}

TEST_F(TickTimerTest, SingleShotTimerTest)
{
	int timerTriggerCount = 0;
	try {
		netpp::eventloop::EventLoop eventLoop;
		auto wheel = std::make_shared<netpp::time::TimeWheel>(&eventLoop, 10, 10);
		netpp::time::TickTimer timer(wheel);
		netpp::time::Timer quitLoopTimer(&eventLoop);

		timer.setOnTimeout([&timerTriggerCount]{ ++timerTriggerCount; });
		timer.setInterval(1);
		timer.setSingleShot(true);
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

TEST_F(TickTimerTest, RepeatedlyTimerTest)
{
	int timerTriggerCount = 0;
	try {
		netpp::eventloop::EventLoop eventLoop;
		auto wheel = std::make_shared<netpp::time::TimeWheel>(&eventLoop, 10, 10);
		netpp::time::TickTimer timer(wheel);

		timer.setOnTimeout([&timerTriggerCount]{ ++timerTriggerCount; if (timerTriggerCount > 5) throw std::runtime_error("quit"); });
		timer.setInterval(1);
		timer.setSingleShot(false);
		timer.start();
		eventLoop.run();
	} catch (const std::runtime_error &e) {
	}
	EXPECT_GT(timerTriggerCount, 5);
}

TEST_F(TickTimerTest, SetSingleShotWhileRunningTest)
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
