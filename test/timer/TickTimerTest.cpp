//
// Created by gaojian on 2022/4/12.
//

#include <gtest/gtest.h>
#include "time/TimeWheel.h"
#include "time/TickTimer.h"
#include "eventloop/EventLoop.h"
#include "Application.h"
#include "eventloop/EventLoopManager.h"

class TickTimerTest : public testing::Test {
public:
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(TickTimerTest, SetTimerPropertyTest)
{
	netpp::Application app;
	netpp::TickTimer timer;

	EXPECT_EQ(timer.interval(), 60);
	EXPECT_EQ(timer.singleShot(), true);
	EXPECT_EQ(timer.running(), false);

	timer.setInterval(2000);
	EXPECT_EQ(timer.interval(), 2000);

	timer.setSingleShot(false);
	EXPECT_EQ(timer.singleShot(), false);
}

TEST_F(TickTimerTest, SingleShotTimerTest)
{
	netpp::Application app;
	int timerTriggerCount = 0;
	netpp::EventLoop *eventLoop = netpp::Application::loopManager()->mainLoop();
	netpp::TickTimer timer(eventLoop);
	netpp::Timer quitLoopTimer(eventLoop);

	timer.setOnTimeout([&timerTriggerCount]{ ++timerTriggerCount; });
	timer.setInterval(1);
	timer.setSingleShot(true);
	quitLoopTimer.setInterval(50);
	quitLoopTimer.setSingleShot(true);
	quitLoopTimer.setOnTimeout([&]{ eventLoop->quit(); });
	timer.start();
	quitLoopTimer.start();
	eventLoop->run();
	EXPECT_EQ(timerTriggerCount, 1);
}

TEST_F(TickTimerTest, RepeatedlyTimerTest)
{
	netpp::Application app;
	int timerTriggerCount = 0;
	netpp::EventLoop *eventLoop = netpp::Application::loopManager()->mainLoop();
	netpp::TickTimer timer;

	timer.setOnTimeout([&]{ ++timerTriggerCount; if (timerTriggerCount > 5) eventLoop->quit(); });
	timer.setInterval(1);
	timer.setSingleShot(false);
	timer.start();
	eventLoop->run();

	EXPECT_GT(timerTriggerCount, 5);
}

TEST_F(TickTimerTest, SetSingleShotWhileRunningTest)
{
	netpp::Application app;
	int timerTriggerCount = 0;
	netpp::EventLoop *eventLoop = netpp::Application::loopManager()->mainLoop();
	netpp::Timer timer(eventLoop);
	netpp::Timer quitLoopTimer(eventLoop);

	timer.setInterval(10);
	timer.setSingleShot(false);
	timer.setOnTimeout([&timerTriggerCount, &timer]{ ++timerTriggerCount; timer.setSingleShot(true); });
	quitLoopTimer.setInterval(50);
	quitLoopTimer.setSingleShot(true);
	quitLoopTimer.setOnTimeout([&]{ eventLoop->quit(); });
	timer.start();
	quitLoopTimer.start();
	eventLoop->run();

	EXPECT_EQ(timerTriggerCount, 2);
}
