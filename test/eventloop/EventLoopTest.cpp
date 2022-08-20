//
// Created by gaojian on 22-8-20.
//

#include <gtest/gtest.h>
#include "eventloop/EventLoop.h"
#include "time/Timer.h"

class EventLoopTest : public testing::Test {
public:
protected:
	void SetUp() override {}

	void TearDown() override {}
};

TEST_F(EventLoopTest, ThisLoopTest)
{
	netpp::EventLoop loop;
	EXPECT_EQ(netpp::EventLoop::thisLoop(), nullptr);
	netpp::Timer loopTaskTimer(&loop);
	loopTaskTimer.setOnTimeout([&]{
		EXPECT_EQ(netpp::EventLoop::thisLoop(), &loop);
		loop.quit();
	});
	loopTaskTimer.setSingleShot(true);
	loopTaskTimer.setInterval(20);
	loopTaskTimer.start();
	loop.run();
	EXPECT_EQ(netpp::EventLoop::thisLoop(), nullptr);
}
