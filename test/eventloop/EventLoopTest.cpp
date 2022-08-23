//
// Created by gaojian on 22-8-20.
//

#include <gtest/gtest.h>
#include <future>
#include "eventloop/EventLoop.h"
#include "eventloop/EventLoopFactory.h"

class EventLoopTest : public testing::Test {
public:
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(EventLoopTest, RunInLoopTest)
{
	auto loop = netpp::EventLoopFactory::makeEventLoop(true, true);
	// run in another thread
	auto task = std::async([&]{
		loop->runInLoop([&]{
			EXPECT_EQ(netpp::EventLoop::thisLoop(), loop.get());
		});
	});
	task.wait();

	// run before loop start
	loop->runInLoop([&]{
		EXPECT_EQ(netpp::EventLoop::thisLoop(), loop.get());
		loop->quit();
	});

	loop->run();
}

TEST_F(EventLoopTest, ThisLoopTest)
{
	auto loop = netpp::EventLoopFactory::makeEventLoop(true, true);
	EXPECT_EQ(netpp::EventLoop::thisLoop(), nullptr);
	loop->runInLoop([&]{
		EXPECT_EQ(netpp::EventLoop::thisLoop(), loop.get());
		loop->quit();
	});
	loop->run();
	EXPECT_EQ(netpp::EventLoop::thisLoop(), nullptr);
}
