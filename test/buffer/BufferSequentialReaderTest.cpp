#include <gtest/gtest.h>
#include "buffer/Buffer.h"
#include "buffer/BufferGather.h"
#include "BufferGatherGetter.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <sys/types.h>
#include <endian.h>
#include <sys/uio.h>
}

using namespace netpp;

class BufferSequentialReaderTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(BufferSequentialReaderTest, ReadInt64)
{
	std::shared_ptr<Buffer> buffer1 = std::make_shared<Buffer>();
	std::shared_ptr<Buffer> buffer2 = std::make_shared<Buffer>();
	uint64_t v{2};
	buffer1->write(reinterpret_cast<const char *>(&v), sizeof(v));
	v = 3;
	buffer2->write(reinterpret_cast<const char *>(&v), sizeof(v));
	SequentialBufferReadGather readVec;
	readVec.addBufferNode(buffer1);
	readVec.addBufferNode(buffer2);
	EXPECT_EQ(get_iovec_len(readVec), 2);
	EXPECT_NE(get_iovec(readVec), nullptr);
	EXPECT_EQ(get_iovec(readVec)[0].iov_len, sizeof(int64_t));
	EXPECT_EQ(get_iovec(readVec)[1].iov_len, sizeof(int64_t));
	readVec.adjustByteArray(sizeof(int64_t) * 2);
	EXPECT_EQ(buffer1->readableBytes(), 0);
	EXPECT_EQ(buffer2->readableBytes(), 0);
}

TEST_F(BufferSequentialReaderTest, ReadInt64FromTwoInt64)
{
	std::shared_ptr<Buffer> buffer1 = std::make_shared<Buffer>();
	std::shared_ptr<Buffer> buffer2 = std::make_shared<Buffer>();
	uint64_t v{2};
	buffer1->write(reinterpret_cast<const char *>(&v), sizeof(v));
	v = 3;
	buffer2->write(reinterpret_cast<const char *>(&v), sizeof(v));
	SequentialBufferReadGather readVec;
	readVec.addBufferNode(buffer1);
	readVec.addBufferNode(buffer2);
	readVec.adjustByteArray(sizeof(int64_t));
	EXPECT_EQ(buffer1->readableBytes(), 0);
	EXPECT_EQ(buffer2->readableBytes(), sizeof(int64_t));
}

TEST_F(BufferSequentialReaderTest, ReadInt8s)
{
	std::shared_ptr<Buffer> buffer1 = std::make_shared<Buffer>();
	std::shared_ptr<Buffer> buffer2 = std::make_shared<Buffer>();
	int8_t v{2};
	buffer1->write(reinterpret_cast<const char *>(&v), sizeof(v));
	v = 3;
	buffer1->write(reinterpret_cast<const char *>(&v), sizeof(v));
	v = 4;
	buffer2->write(reinterpret_cast<const char *>(&v), sizeof(v));

	SequentialBufferReadGather readVec;
	readVec.addBufferNode(buffer1);
	readVec.addBufferNode(buffer2);

	EXPECT_EQ(get_iovec_len(readVec), 2);
	EXPECT_NE(get_iovec(readVec), nullptr);
	EXPECT_EQ(get_iovec(readVec)[0].iov_len, sizeof(int8_t) * 2);
	EXPECT_EQ(get_iovec(readVec)[1].iov_len, sizeof(int8_t));
	readVec.adjustByteArray(sizeof(int8_t) * 3);

	EXPECT_EQ(buffer1->readableBytes(), 0);
	EXPECT_EQ(buffer2->readableBytes(), 0);
}
