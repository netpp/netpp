//
// Created by gaojian on 2022/4/12.
//

#include <gtest/gtest.h>
#define private public
#define protected public
#include "time/TickTimer.h"
#include "time/TimeWheel.h"
#include "eventloop/EventLoop.h"
#undef private
#undef protected

using namespace netpp::time;

class TimerWheelTest : public testing::Test {
public:
protected:
	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(TimerWheelTest, WheelRotateTest)
{
	netpp::eventloop::EventLoop loop;
	TimeWheel wheel(&loop, 10, 10);
	EXPECT_EQ(wheel.m_timeOutBucketIndex, 0);
	for (int i = 1; i < 10; ++i)
	{
		wheel.tick();
		EXPECT_EQ(wheel.m_timeOutBucketIndex, i);
	}
	wheel.tick();
	EXPECT_EQ(wheel.m_timeOutBucketIndex, 0);
}

TEST_F(TimerWheelTest, AddEntryTest)
{
	netpp::eventloop::EventLoop loop;
	TimeWheel wheel(&loop, 10, 10);
	{
		auto entry = std::make_shared<WheelEntry>();
		entry->timeoutTick = 1;
		wheel.addToWheel(entry);
		EXPECT_EQ(entry->wheelIndex, 1);
		EXPECT_EQ(entry->expire, false);
	}
	{
		auto entry = std::make_shared<WheelEntry>();
		entry->expire = true;
		entry->timeoutTick = 1;
		wheel.addToWheel(entry);
		EXPECT_EQ(entry->wheelIndex, 1);
		EXPECT_EQ(entry->expire, false);
	}
	{
		auto entry = std::make_shared<WheelEntry>();
		entry->timeoutTick = 0;
		wheel.addToWheel(entry);
		EXPECT_EQ(entry->wheelIndex, 1);
		EXPECT_EQ(entry->expire, false);
	}
	{
		auto entry = std::make_shared<WheelEntry>();
		entry->timeoutTick = 10;
		wheel.addToWheel(entry);
		EXPECT_EQ(entry->wheelIndex, 1);
		EXPECT_EQ(entry->expire, false);
	}
	{
		auto entry = std::make_shared<WheelEntry>();
		entry->timeoutTick = 11;
		wheel.addToWheel(entry);
		EXPECT_EQ(entry->wheelIndex, 2);
		EXPECT_EQ(entry->expire, false);
	}
}

TEST_F(TimerWheelTest, RemoveEntryTest)
{
	netpp::eventloop::EventLoop loop;
	TimeWheel wheel(&loop, 10, 2);

	auto entry1 = std::make_shared<WheelEntry>();
	entry1->timeoutTick = 1;
	entry1->singleShot = false;
	entry1->callback = [&]{
		static int i = 0;
		++i;
		if (i > 2)
		{
			wheel.removeFromWheel(entry1);
			EXPECT_TRUE(entry1->expire);
			EXPECT_EQ(wheel.m_buckets[1].size(), 3);
		}
	};
	wheel.addToWheel(entry1);
	auto entry2 = std::make_shared<WheelEntry>();
	entry2->singleShot = true;
	entry2->timeoutTick = 1;
	wheel.addToWheel(entry2);
	auto entry3 = std::make_shared<WheelEntry>();
	entry3->singleShot = false;
	entry3->timeoutTick = 1;
	wheel.addToWheel(entry3);
	EXPECT_EQ(wheel.m_buckets[1].size(), 3);

	wheel.tick();
	EXPECT_EQ(wheel.m_buckets[1].size(), 2);

	wheel.tick();
	EXPECT_EQ(wheel.m_buckets[1].size(), 2);

	wheel.tick();
	EXPECT_EQ(wheel.m_buckets[1].size(), 1);
}

TEST_F(TimerWheelTest, RenewEntryTest)
{
	netpp::eventloop::EventLoop loop;
	TimeWheel wheel(&loop, 10, 10);

	auto entry = std::make_shared<WheelEntry>();
	entry->timeoutTick = 1;

	wheel.addToWheel(entry);
	EXPECT_EQ(entry->wheelIndex, 1);
	EXPECT_EQ(wheel.m_buckets[1].size(), 1);

	entry->timeoutTick = 3;
	EXPECT_EQ(entry->wheelIndex, 3);
	EXPECT_EQ(wheel.m_buckets[1].size(), 0);
	EXPECT_EQ(wheel.m_buckets[3].size(), 1);

	entry->timeoutTick = 0;
	EXPECT_EQ(entry->wheelIndex, 1);
	EXPECT_EQ(wheel.m_buckets[1].size(), 1);
	EXPECT_EQ(wheel.m_buckets[3].size(), 0);

	entry->timeoutTick = 10;
	EXPECT_EQ(entry->wheelIndex, 1);
	EXPECT_EQ(wheel.m_buckets[1].size(), 1);
	EXPECT_EQ(wheel.m_buckets[3].size(), 0);
}

TEST_F(TimerWheelTest, TickTest)
{
	netpp::eventloop::EventLoop loop;
	TimeWheel wheel(&loop, 10, 3);

	int triggerCount = 0;
	auto entry = std::make_shared<WheelEntry>();
	entry->timeoutTick = 1;
	entry->callback = [&triggerCount]{
		++triggerCount;
	};
	wheel.addToWheel(entry);

	wheel.tick();
	EXPECT_EQ(triggerCount, 1);

	wheel.tick();
	EXPECT_EQ(triggerCount, 1);

	wheel.tick();
	EXPECT_EQ(triggerCount, 1);

	wheel.tick();
	EXPECT_EQ(triggerCount, 2);
}
