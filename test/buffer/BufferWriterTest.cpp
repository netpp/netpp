#include <gtest/gtest.h>
#include <cstring>
#include "buffer/Buffer.h"
#include "buffer/BufferGather.h"
#include "BufferGatherGetter.h"
extern "C" {
#include <sys/uio.h>
}

using namespace netpp;

class ByteArrayWriterTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}

	static void writeToIOVec(::iovec *vec, int index, const void *data, std::size_t length)
	{
		std::memcpy(vec[index].iov_base, data, length);
	}
};

TEST_F(ByteArrayWriterTest, EmptyByteArray)
{
	BufferWriteGather writeVec;
	EXPECT_EQ(get_iovec_len(writeVec), 2);
	EXPECT_NE(get_iovec(writeVec), nullptr);
	EXPECT_EQ(get_iovec(writeVec)[0].iov_len, Buffer::DefaultBufferSize);
	writeVec.adjustByteArray(0);
	EXPECT_EQ(writeVec.getBuffer()->readableBytes(), 0);
}

TEST_F(ByteArrayWriterTest, WriteOutOfRange)
{
	BufferWriteGather writeVec;
	writeVec.adjustByteArray(Buffer::DefaultBufferSize * 2 + 1);
	EXPECT_EQ(writeVec.getBuffer()->readableBytes(), Buffer::DefaultBufferSize * 2 + 1);
}

TEST_F(ByteArrayWriterTest, WriteInt8)
{
	BufferWriteGather writeVec;
	int8_t value = 1;
	writeToIOVec(get_iovec(writeVec), 0, &value, sizeof(int8_t));
	writeVec.adjustByteArray(sizeof(int8_t));
	auto buffer = writeVec.getBuffer();
	EXPECT_EQ(buffer->readableBytes(), sizeof(int8_t));
	int8_t v = 0;
	buffer->retrieve(reinterpret_cast<char *>(&v), sizeof(int8_t));
	EXPECT_EQ(v, 1);
}

TEST_F(ByteArrayWriterTest, WriteInt64)
{
	BufferWriteGather writeVec;
	int64_t value = 2;
	writeToIOVec(get_iovec(writeVec), 0, &value, sizeof(int64_t));
	writeVec.adjustByteArray(sizeof(int64_t));
	auto buffer = writeVec.getBuffer();
	EXPECT_EQ(buffer->readableBytes(), sizeof(int64_t));
	int64_t v = 0;
	buffer->retrieve(reinterpret_cast<char *>(&v), sizeof(int64_t));
	EXPECT_EQ(v, 2);
}
