//
// Created by gaojian on 22-8-22.
//

#include <gtest/gtest.h>
#include <buffer/TcpBuffer.h>

class ChannelBufferTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(ChannelBufferTest, TcpBufferRead)
{
	netpp::TcpBuffer buffer;
	EXPECT_EQ(buffer.readableBytes(), 0);
}

TEST_F(ChannelBufferTest, TcpBufferWrite)
{}

TEST_F(ChannelBufferTest, TcpBufferReadConvert)
{}

TEST_F(ChannelBufferTest, TcpBufferWriteConvert)
{}
