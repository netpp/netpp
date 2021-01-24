#include <gtest/gtest.h>
#include "time/TimeWheel.h"
#include "EventLoop.h"
#include "support/ThreadPool.hpp"

// TODO: can not trigger timerfd in gtest, why??
class TimerTest : public testing::Test {
public:
	static int timerTriggerCount;
	static int wheelTriggerCount;
protected:
	void SetUp() override { timerTriggerCount = 0; }
	void TearDown() override { wheelTriggerCount = 0; }
};
int TimerTest::timerTriggerCount = 0;
int TimerTest::wheelTriggerCount = 0;

TEST_F(TimerTest, Timer)
{
	/*netpp::EventLoop loop;
	std::shared_ptr<netpp::time::Timer> timer;
	loop.runInLoop([&]{
		timer = std::make_shared<netpp::time::Timer>(&loop);
		timer->setInterval(1000);
		timer->setSingleShot(false);
		timer->setOnTimeout([]{
			++timerTriggerCount;
			if (timerTriggerCount == 5);
				throw std::runtime_error("");
		});
		timer->start();
	});
	loop.run();
	EXPECT_THROW(loop.run(), std::runtime_error);
	EXPECT_EQ(timerTriggerCount, 3);*/
}

TEST_F(TimerTest, TimeWheel)
{
	/*class TestWheelEntry : public netpp::time::TimeWheelEntry {
	public:
		~TestWheelEntry() override { ++wheelTriggerCount; }
	};
	netpp::EventLoop loop;
	std::shared_ptr<netpp::time::Timer> timer;
	std::shared_ptr<netpp::time::TimeWheel> wheel;
	loop.runInLoop([&]{
		timer = std::make_shared<netpp::time::Timer>(&loop);
		timer->setInterval(3500);
		timer->setSingleShot(true);
		timer->setOnTimeout([]{ throw std::runtime_error(""); });
		timer->start();
		wheel = std::make_shared<netpp::time::TimeWheel>(&loop, 1, 2);
		wheel->addToWheel(std::make_shared<TestWheelEntry>());
	});
	EXPECT_THROW(loop.run(), std::runtime_error);
	EXPECT_EQ(wheelTriggerCount, 3);*/
}
