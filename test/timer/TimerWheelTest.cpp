//
// Created by gaojian on 2022/4/12.
//

#include <gtest/gtest.h>
#include "time/TimeWheel.h"
#include "eventloop/EventLoop.h"

class TimerWheelTest : public testing::Test {
public:
protected:
	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(TimerWheelTest, AddEntryTest)
{
	netpp::EventLoop loop;
	netpp::TimeWheel wheel(&loop);
	{
		netpp::TimeWheel::WheelEntryData entry;
		void *fakeTimerId = reinterpret_cast<void *>(1);
		entry.interval = 1000;
		wheel.addToWheel(fakeTimerId, entry);
		EXPECT_EQ(entry.wheelIndexer.secondWheel, 0);
		EXPECT_EQ(entry.wheelIndexer.minuteWheel, 0);
		EXPECT_EQ(entry.wheelIndexer.hourWheel, 0);
		EXPECT_EQ(entry.wheelIndexer.dayWheel, 0);
		EXPECT_EQ(entry.expire, false);
		EXPECT_EQ(entry.timerId, fakeTimerId);
	}
	{
		netpp::TimeWheel::WheelEntryData entry;
		void *fakeTimerId = reinterpret_cast<void *>(2);
		entry.expire = true;
		entry.interval = 1000;
		wheel.addToWheel(fakeTimerId, entry);
		EXPECT_EQ(entry.expire, false);
	}
	{
		netpp::TimeWheel::WheelEntryData entry;
		void *fakeTimerId = reinterpret_cast<void *>(3);
		entry.interval = 0;
		wheel.addToWheel(fakeTimerId, entry);
		EXPECT_EQ(entry.wheelIndexer.secondWheel, 0);
		EXPECT_EQ(entry.wheelIndexer.minuteWheel, 0);
		EXPECT_EQ(entry.wheelIndexer.hourWheel, 0);
		EXPECT_EQ(entry.wheelIndexer.dayWheel, 0);
	}
	{
		netpp::TimeWheel::WheelEntryData entry;
		void *fakeTimerId = reinterpret_cast<void *>(4);
		entry.interval = 10000;
		wheel.addToWheel(fakeTimerId, entry);
		EXPECT_EQ(entry.wheelIndexer.secondWheel, 9);
		EXPECT_EQ(entry.wheelIndexer.minuteWheel, 0);
		EXPECT_EQ(entry.wheelIndexer.hourWheel, 0);
		EXPECT_EQ(entry.wheelIndexer.dayWheel, 0);
	}
}

TEST_F(TimerWheelTest, RemoveEntryTest)
{
	netpp::EventLoop loop;
	netpp::TimeWheel wheel(&loop);

	netpp::TimeWheel::WheelEntryData entry1;
	void *fakeTimerId1 = reinterpret_cast<void *>(1);
	entry1.interval = 1000;
	entry1.singleShot = false;
	entry1.callback = [&]{
		static int i = 0;
		++i;
		if (i >= 2)
		{
			wheel.removeFromWheel(entry1);
			EXPECT_TRUE(entry1.expire);
		}
	};
	wheel.addToWheel(fakeTimerId1, entry1);

	netpp::TimeWheel::WheelEntryData entry2;
	void *fakeTimerId2 = reinterpret_cast<void *>(2);
	entry2.singleShot = true;
	entry2.interval = 1000;
	wheel.addToWheel(fakeTimerId2, entry2);

	netpp::TimeWheel::WheelEntryData entry3;
	void *fakeTimerId3 = reinterpret_cast<void *>(3);
	entry3.singleShot = false;
	entry3.interval = 1000;
	wheel.addToWheel(fakeTimerId3, entry3);

	// wait after all entry timed out
}

TEST_F(TimerWheelTest, RenewEntryTest)
{
	netpp::EventLoop loop;
	netpp::TimeWheel wheel(&loop);

	netpp::TimeWheel::WheelEntryData entry;
	void *fakeTimerId = reinterpret_cast<void *>(1);
	entry.interval = 1000;

	wheel.addToWheel(fakeTimerId, entry);
	EXPECT_EQ(entry.wheelIndexer.secondWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.minuteWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.hourWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.dayWheel, 0);

	entry.interval = 3000;
	wheel.renew(entry);
	EXPECT_EQ(entry.wheelIndexer.secondWheel, 2);
	EXPECT_EQ(entry.wheelIndexer.minuteWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.hourWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.dayWheel, 0);

	entry.interval = 0;
	wheel.renew(entry);
	EXPECT_EQ(entry.wheelIndexer.secondWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.minuteWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.hourWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.dayWheel, 0);

	entry.interval = 10000;
	wheel.renew(entry);
	EXPECT_EQ(entry.wheelIndexer.secondWheel, 9);
	EXPECT_EQ(entry.wheelIndexer.minuteWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.hourWheel, 0);
	EXPECT_EQ(entry.wheelIndexer.dayWheel, 0);
}

TEST_F(TimerWheelTest, TickTest)
{
	netpp::EventLoop loop;
	netpp::TimeWheel wheel(&loop);
	netpp::Timer quitLoopTimer(&loop);

	int triggerCount = 0;
	netpp::TimeWheel::WheelEntryData entry;
	void *fakeTimerId = reinterpret_cast<void *>(1);
	entry.interval = 1000;
	entry.singleShot = false;
	entry.callback = [&triggerCount]{
		++triggerCount;
	};
	wheel.addToWheel(fakeTimerId, entry);
	quitLoopTimer.setInterval(2500);
	quitLoopTimer.setSingleShot(true);
	quitLoopTimer.setOnTimeout([&]{ loop.quit(); });
	quitLoopTimer.start();

	loop.run();

	// wait util entry timed out
	EXPECT_EQ(triggerCount, 2);
}
