//
// Created by gaojian on 2022/10/8.
//

#include <gtest/gtest.h>
#include <cstring>
#include "buffer/Buffer.h"

using namespace netpp;

class BufferTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(BufferTest, DefaultBuffer)
{
	Buffer buffer;
	EXPECT_EQ(buffer.readableBytes(), 0);
	EXPECT_EQ(buffer.writeableBytes(), Buffer::DefaultBufferSize);
	EXPECT_NE(buffer.getRawPointer(), nullptr);
}

TEST_F(BufferTest, WriteIntoBuffer)
{
	Buffer buffer;
	std::string str("some test string");
	buffer.write(str.data(), str.length());
	EXPECT_EQ(buffer.readableBytes(), str.length());
}

TEST_F(BufferTest, MoveBufferData)
{
	Buffer buffer;
	constexpr BufferLength BufferSize = Buffer::DefaultBufferSize - 10;
	char array[BufferSize];
	char copy[BufferSize];
	buffer.write(array, BufferSize);
	buffer.retrieve(copy, BufferSize / 2);
	// now data would move to head
	buffer.write(array, 10);
	EXPECT_EQ(buffer.readableBytes(), BufferSize / 2 + 10);
}

TEST_F(BufferTest, WriteLargeEntryIntoBuffer)
{
	Buffer buffer;
	char reallyLargeArray[Buffer::DefaultBufferSize * 10];
	for (int i = 0; i < 10; ++i)
		buffer.write(reallyLargeArray, Buffer::DefaultBufferSize * 10);
	EXPECT_EQ(buffer.readableBytes(), Buffer::DefaultBufferSize * 10 * 10);
}

TEST_F(BufferTest, ReadFromEmptyBuffer)
{
	Buffer buffer;
	char buf[10] = "";
	BufferLength size = buffer.retrieve(buf, 10);
	EXPECT_EQ(size, 0);
	EXPECT_STREQ(buf, "");
}

TEST_F(BufferTest, ReadFromBuffer)
{
	Buffer buffer;
	std::string str("some test string");
	auto length = str.length();
	buffer.write(str.c_str(), length);
	char buf[100] = "";
	std::memset(buf, 0, sizeof(char) * 100);
	BufferLength size = buffer.retrieve(buf, length);
	EXPECT_EQ(size, length);
	EXPECT_STREQ(buf, str.c_str());
}

TEST_F(BufferTest, ReadGreaterSizeThanBuffer)
{
	Buffer buffer;
	std::string str("some test string");
	auto length = str.length();
	buffer.write(str.c_str(), length);
	char buf[100] = "";
	std::memset(buf, 0, sizeof(char) * 100);
	BufferLength size = buffer.retrieve(buf, sizeof(char) * 100);
	EXPECT_EQ(size, length);
	EXPECT_STREQ(buf, str.c_str());
}

TEST_F(BufferTest, GetRawBuffer)
{}

TEST_F(BufferTest, UpdateBufferSize)
{
	Buffer buffer;
	EXPECT_EQ(buffer.readableBytes(), 0);
	buffer.writeInSize(20);
	EXPECT_EQ(buffer.readableBytes(), 20);
	buffer.readOutSize(5);
	EXPECT_EQ(buffer.readableBytes(), 15);
}
