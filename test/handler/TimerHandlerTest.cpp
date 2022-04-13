//
// Created by gaojian on 2022/4/12.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#define private public
#define protected public
#include "eventloop/EventLoop.h"
#include "internal/handlers/TimerHandler.h"
#undef private
#undef protected

using namespace netpp::internal::handlers;

class TimerHandlerTest : public testing::Test {
public:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(TimerHandlerTest, HandleEventTest)
{
	netpp::eventloop::EventLoop loop;
	auto timerHandler = std::make_shared<TimerHandler>(&loop);
	EXPECT_NE(timerHandler->m_timerFd, -1);
}
