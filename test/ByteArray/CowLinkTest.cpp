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

TEST_F(CowLinkTest, Iteration)
{
	CowBuffer link;
	CowBuffer::NodeContainerIndexer index = 0;
	for (auto it = link.cowBegin(0); (it <=> link.cowEnd(CowBuffer::defaultNodeSize)) < 0; ++it, ++index)
	{
		EXPECT_EQ(it->buffer, (*it).buffer);
		EXPECT_TRUE(link.m_nodes[index].unique());
	}
}

TEST_F(CowLinkTest, CowIteration)
{
	CowBuffer link1;
	CowBuffer link2(link1);
	EXPECT_EQ(reinterpret_cast<const void *>(link1.constBegin(0)->buffer), reinterpret_cast<void *>(link1.m_nodes[0]->buffer));
	EXPECT_EQ(reinterpret_cast<const void *>(link2.constBegin(0)->buffer), reinterpret_cast<void *>(link2.m_nodes[0]->buffer));
	EXPECT_EQ(reinterpret_cast<const void *>(link1.constBegin(0)->buffer), reinterpret_cast<const void *>(link2.constBegin(0)->buffer));
	CowBuffer::NodeContainerIndexer index = 0;
	for (auto it = link1.cowBegin(0); (it <=> link1.cowEnd(CowBuffer::defaultNodeSize)) < 0; ++it, ++index)
	{
		EXPECT_NE(link1.m_nodes[index], link2.m_nodes[index]);
		EXPECT_NE(reinterpret_cast<const void *>(link1.constBegin(0)->buffer), reinterpret_cast<const void *>(link2.constBegin(0)->buffer));
		EXPECT_EQ(it->buffer, (*it).buffer);
		EXPECT_TRUE(link1.m_nodes[index].unique());
		EXPECT_TRUE(link2.m_nodes[index].unique());
	}
}

TEST_F(CowLinkTest, AllocMore)
{
	CowBuffer link;
	link.allocMore(10);
	EXPECT_EQ(link.m_nodes.size(), 10);
}

TEST_F(CowLinkTest, AllocBy0)
{
	CowBuffer link;
	link.allocMore(0);
	EXPECT_EQ(link.m_nodes.size(), CowBuffer::defaultNodeSize);
}

TEST_F(CowLinkTest, MoveNode)
{
	CowBuffer link;
	CowBuffer::NodeContainerIndexer size = 10;
	link.allocMore(size);
	auto nodes = link.m_nodes;
	CowBuffer::NodeContainerIndexer end = 5;
	link.moveToTail(end, [](BufferNode &node) { node.start = 0; node.end = 0; });
	for (CowBuffer::NodeContainerIndexer i = 0; i < end; ++i)
	{
		EXPECT_EQ(link.m_nodes[i]->start, 0);
		EXPECT_EQ(link.m_nodes[i]->end, 0);
		EXPECT_EQ(link.m_nodes[i].get(), nodes[i + end].get());
	}
	for (CowBuffer::NodeContainerIndexer i = end; i < size; ++i)
		EXPECT_EQ(link.m_nodes[i].get(), nodes[i - end].get());
}

TEST_F(CowLinkTest, MoveNodeBy0)
{
	CowBuffer link;
	CowBuffer::NodeContainerIndexer size = 10;
	link.allocMore(size);
	auto nodes = link.m_nodes;
	link.moveToTail(0, [](BufferNode &node) { node.start = 0; node.end = 0; });
	for (CowBuffer::NodeContainerIndexer i = 0; i < size; ++i)
	{
		EXPECT_EQ(link.m_nodes[i]->start, 0);
		EXPECT_EQ(link.m_nodes[i]->end, 0);
		EXPECT_EQ(link.m_nodes[i].get(), nodes[i].get());
	}
}
