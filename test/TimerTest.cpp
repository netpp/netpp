#include <gtest/gtest.h>
#include "time/TimeWheel.h"
#include "EventLoop.h"
#include "support/ThreadPool.hpp"

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

class TestWheelEntry : public netpp::time::TimeWheelEntry {
public:
	explicit TestWheelEntry(std::weak_ptr<netpp::time::TimeWheel> wheel) : _wheel{wheel} {}
	~TestWheelEntry() override = default;
	void onTimeout() override;
private:
	std::weak_ptr<netpp::time::TimeWheel> _wheel;
};

void TestWheelEntry::onTimeout()
{
	_wheel.lock()->addToWheel(std::make_shared<TestWheelEntry>(_wheel.lock()));
	++TimerTest::wheelTriggerCount;
}

TEST_F(TimerTest, TimeWheel)
{
	netpp::EventLoop loop;
	std::shared_ptr<netpp::time::Timer> timer;
	std::shared_ptr<netpp::time::TimeWheel> wheel;
	loop.runInLoop([&]{
		timer = std::make_shared<netpp::time::Timer>(&loop);
		timer->setInterval(430);
		timer->setSingleShot(true);
		timer->setOnTimeout([]{ throw std::runtime_error(""); });
		timer->start();
		wheel = std::make_shared<netpp::time::TimeWheel>(&loop, 50, 2);
		wheel->addToWheel(std::make_shared<TestWheelEntry>(wheel));
	});
	EXPECT_THROW(loop.run(), std::runtime_error);
	EXPECT_EQ(wheelTriggerCount, 8);
}
