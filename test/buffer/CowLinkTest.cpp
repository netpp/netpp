//
// Created by gaojian on 2022/2/27.
//

#include <gtest/gtest.h>
#include "buffer/CowLink.hpp"
#include "buffer/BufferNode.h"
#include <cstring>

using namespace netpp;
using CowBuffer = CowLink<BufferNode>;

class CowLinkTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(CowLinkTest, CreateCowBuffer)
{
	CowBuffer link;
	EXPECT_EQ(link.size(), CowBuffer::defaultNodeSize);
}

TEST_F(CowLinkTest, CopyLInk)
{
	CowBuffer link1;
	std::string testBuffer("test buffer");
	std::memcpy(link1.begin()->buffer, testBuffer.data(), testBuffer.length());
	CowBuffer link2(link1);
	EXPECT_EQ(link1.size(), link2.size());
	EXPECT_EQ(link1.begin()->start, link2.begin()->start);
	EXPECT_EQ(link1.begin()->end, link2.begin()->end);
	EXPECT_STREQ(link1.begin()->buffer, link2.begin()->buffer);

	std::string testBuffer2("test buffer2");
	std::memcpy(link2.begin()->buffer, testBuffer.data(), testBuffer.length());
	EXPECT_STRNE(link1.begin()->buffer, link2.begin()->buffer);
	EXPECT_STREQ(link1.begin()->buffer, testBuffer.c_str());
	EXPECT_STREQ(link2.begin()->buffer, testBuffer2.c_str());
}

TEST_F(CowLinkTest, RangBasedIteration)
{
	CowBuffer link;
	link.allocMore(10);
	unsigned i = 0;
	for (auto &node : link)
	{
		node.start = i;
		node.end = i + 1;
		++i;
	}
	CowBuffer copiedLink(link);
	i = 0;
	for (auto &node : link)
	{
		EXPECT_EQ(node.start, i);
		EXPECT_EQ(node.end, i + 1);
		node.start = i + 2;
		node.end = i + 3;
		++i;
	}
	i = 0;
	for (auto &node : copiedLink)
	{
		EXPECT_EQ(node.start, i);
		EXPECT_EQ(node.end, i + 1);
		++i;
	}
}

TEST_F(CowLinkTest, RangedIteration)
{
	CowBuffer link;
	link.allocMore(10);
	unsigned i = 0;
	for (auto &it : link)
	{
		it.start = i;
		it.end = i + 1;
		++i;
	}
	// include first
	i = 3;
	for (auto &it : link.range(3, 7))
	{
		EXPECT_EQ(it.start, i);
		EXPECT_EQ(it.end, i + 1);
		++i;
	}
	// not include last
	EXPECT_EQ(i, 6);
}

TEST_F(CowLinkTest, CustomStepIteration)
{
	CowBuffer link;
	link.allocMore(10);
	unsigned i = 0;
	for (auto &it : link)
	{
		it.start = i;
		it.end = i + 1;
		++i;
	}
	CowBuffer copiedLink(link);
	i = 0;
	for (auto it = link.begin(); it != link.end(); it += 2, i += 2)
	{
		EXPECT_EQ(it->start, i);
		EXPECT_EQ(it->end, i + 1);
		it->start = i + 2;
		it->end = i + 3;
	}
	for (auto &node : copiedLink)
	{
		EXPECT_EQ(node.start, i);
		EXPECT_EQ(node.end, i + 1);
		++i;
	}
}

TEST_F(CowLinkTest, AllocMore)
{
	CowBuffer link;
	link.allocMore(10);
	EXPECT_EQ(link.size(), 10);
}

TEST_F(CowLinkTest, AllocBy0)
{
	CowBuffer link;
	link.allocMore(0);
	EXPECT_EQ(link.size(), 0);
}

TEST_F(CowLinkTest, MoveNode)
{
	CowBuffer link;
	link.allocMore(10);
	unsigned i = 0;
	for (auto &it : link)
	{
		it.start = i;
		it.end = i + 1;
		++i;
	}
	auto it = link.begin();
	it += 5;
	link.moveToTail(it);

	it = link.begin();
	for (unsigned v = 5; v < 10; ++v, ++it)
	{
		EXPECT_EQ(it->start, v);
		EXPECT_EQ(it->end, v + 1);
	}
	for (unsigned v = 0; v < 5; ++v, ++it)
	{
		EXPECT_EQ(it->start, v);
		EXPECT_EQ(it->end, v + 1);
	}
}

TEST_F(CowLinkTest, MoveNodeBy0)
{
	CowBuffer link;
	link.allocMore(10);
	unsigned i = 0;
	for (auto &it : link)
	{
		it.start = i;
		it.end = i + 1;
		++i;
	}
	link.moveToTail(link.begin());
	i = 0;
	for (auto &node : link)
	{
		EXPECT_EQ(node.start, i);
		EXPECT_EQ(node.end, i + 1);
	}
}
