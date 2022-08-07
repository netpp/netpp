//
// Created by gaojian on 22-8-6.
//

#include <gtest/gtest.h>
#include "buffer/BufferNode.h"
#include <cstring>

class BufferNodeTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(BufferNodeTest, Creation)
{
	netpp::BufferNode node;
	EXPECT_EQ(node.start, 0);
	EXPECT_EQ(node.end, 0);

	char buffer[netpp::BufferNode::BufferNodeSize];
	for (auto i = 0; i < 100; ++i)
		buffer[i] = static_cast<char>(i);
	std::memcpy(node.buffer, buffer, netpp::BufferNode::BufferNodeSize);
	char emptyBuffer[netpp::BufferNode::BufferNodeSize];
	for (auto i = 0; i < 100; ++i)
		buffer[i] = static_cast<char>(0);
	std::memcpy(emptyBuffer, node.buffer, netpp::BufferNode::BufferNodeSize);
	for (auto i = 0; i < 100; ++i)
	{
		EXPECT_EQ(emptyBuffer[i], i);
	}
}

TEST_F(BufferNodeTest, CopyNode)
{
	netpp::BufferNode node;
	node.start = 2;
	node.end = 40;
	char buffer[netpp::BufferNode::BufferNodeSize];
	for (auto i = 0; i < 100; ++i)
		buffer[i] = static_cast<char>(i);
	std::memcpy(node.buffer, buffer, netpp::BufferNode::BufferNodeSize);

	netpp::BufferNode newNode(node);
	EXPECT_EQ(newNode.start, node.start);
	EXPECT_EQ(newNode.end, node.start);
	EXPECT_NE(newNode.buffer, node.buffer);
	for (auto i = 0; i < 100; ++i)
	{
		EXPECT_EQ(newNode.buffer[i], i);
	}
}
