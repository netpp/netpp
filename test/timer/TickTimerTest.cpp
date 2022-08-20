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
	netpp::Application app;
	int timerTriggerCount = 0;
	netpp::EventLoop *eventLoop = netpp::Application::loopManager()->mainLoop();
	netpp::TickTimer timer(eventLoop);
	netpp::Timer quitLoopTimer(eventLoop);

	timer.setOnTimeout([&timerTriggerCount]{ ++timerTriggerCount; });
	timer.setInterval(1000);
	timer.setSingleShot(true);
	quitLoopTimer.setInterval(1100);
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

	timer.setOnTimeout([&]{ ++timerTriggerCount; if (timerTriggerCount > 1) eventLoop->quit(); });
	timer.setInterval(1000);
	timer.setSingleShot(false);
	timer.start();
	eventLoop->run();

	EXPECT_EQ(timerTriggerCount, 2);
}

TEST_F(TickTimerTest, SetSingleShotWhileRunningTest)
{
	netpp::Application app;
	int timerTriggerCount = 0;
	netpp::EventLoop *eventLoop = netpp::Application::loopManager()->mainLoop();
	netpp::TickTimer timer(eventLoop);
	netpp::Timer quitLoopTimer(eventLoop);

	timer.setInterval(1000);
	timer.setSingleShot(false);
	timer.setOnTimeout([&timerTriggerCount, &timer]{ ++timerTriggerCount; timer.setSingleShot(true); });
	quitLoopTimer.setInterval(2100);
	quitLoopTimer.setSingleShot(true);
	quitLoopTimer.setOnTimeout([&]{ eventLoop->quit(); });
	timer.start();
	quitLoopTimer.start();
	eventLoop->run();

	EXPECT_EQ(timerTriggerCount, 1);
}

TEST_F(TickTimerTest, RestartOnTimeoutTest)
{
	netpp::Application app;
	int timerTriggerCount = 0;
	netpp::EventLoop *eventLoop = netpp::Application::loopManager()->mainLoop();
	netpp::TickTimer timer(eventLoop);
	netpp::Timer quitLoopTimer(eventLoop);

	timer.setInterval(1000);
	timer.setSingleShot(true);
	timer.setOnTimeout([&timerTriggerCount, &timer]{ ++timerTriggerCount; timer.restart(); });
	quitLoopTimer.setInterval(2100);
	quitLoopTimer.setSingleShot(true);
	quitLoopTimer.setOnTimeout([&]{ eventLoop->quit(); });
	timer.start();
	quitLoopTimer.start();
	eventLoop->run();

	EXPECT_EQ(timerTriggerCount, 2);
}

TEST_F(TickTimerTest, StopOnTimeoutTest)
{
	netpp::Application app;
	int timerTriggerCount = 0;
	netpp::EventLoop *eventLoop = netpp::Application::loopManager()->mainLoop();
	netpp::TickTimer timer(eventLoop);
	netpp::Timer quitLoopTimer(eventLoop);

	timer.setInterval(1000);
	timer.setSingleShot(false);
	timer.setOnTimeout([&timerTriggerCount, &timer]{ ++timerTriggerCount; timer.stop(); });
	quitLoopTimer.setInterval(2100);
	quitLoopTimer.setSingleShot(true);
	quitLoopTimer.setOnTimeout([&]{ eventLoop->quit(); });
	timer.start();
	quitLoopTimer.start();
	eventLoop->run();

	EXPECT_EQ(timerTriggerCount, 1);
}

TEST_F(TickTimerTest, DeleteOnTimeoutTest)
{
	netpp::Application app;
	int timerTriggerCount = 0;
	netpp::EventLoop *eventLoop = netpp::Application::loopManager()->mainLoop();
	auto *timer = new netpp::TickTimer(eventLoop);
	netpp::Timer quitLoopTimer(eventLoop);

	timer->setInterval(1000);
	timer->setSingleShot(false);
	timer->setOnTimeout([&]{ ++timerTriggerCount; delete timer; });
	quitLoopTimer.setInterval(2100);
	quitLoopTimer.setSingleShot(true);
	quitLoopTimer.setOnTimeout([&]{ eventLoop->quit(); });
	timer->start();
	quitLoopTimer.start();
	eventLoop->run();

	EXPECT_EQ(timerTriggerCount, 1);
}
