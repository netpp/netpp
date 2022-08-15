#include <gtest/gtest.h>
#include <cstring>
#include "buffer/ByteArray.h"
#include "buffer/BufferIOConversion.h"
extern "C" {
#include <sys/socket.h>
#include <sys/types.h>
#include <endian.h>
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
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	{
		ByteArrayWriterWithLock writeVec(byteArray);
		EXPECT_EQ(writeVec.iovenLength(), 1);
		EXPECT_NE(writeVec.iovec(), nullptr);
		EXPECT_EQ(writeVec.iovec()[0].iov_len, ByteArray::BufferNodeSize);
		writeVec.adjustByteArray(0);
	}
	EXPECT_EQ(byteArray->readableBytes(), 0);
}

TEST_F(ByteArrayWriterTest, WriteOutOfRange)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	{
		ByteArrayWriterWithLock writeVec(byteArray);
		EXPECT_EQ(writeVec.iovenLength(), 1);
		EXPECT_NE(writeVec.iovec(), nullptr);
		EXPECT_EQ(writeVec.iovec()[0].iov_len, ByteArray::BufferNodeSize);
		writeVec.adjustByteArray(ByteArray::BufferNodeSize + 1);
	}
//	EXPECT_EQ(byteArray->m_nodes->size(), 2);
	EXPECT_EQ(byteArray->readableBytes(), ByteArray::BufferNodeSize + 1);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - 1);
}

TEST_F(ByteArrayWriterTest, WriteInt8)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	{
		ByteArrayWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.iovec();
		EXPECT_EQ(writeVec.iovenLength(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, ByteArray::BufferNodeSize);
		int8_t value = 1;
		writeToIOVec(writeVec.iovec(), 0, &value, sizeof(int8_t));
		writeVec.adjustByteArray(sizeof(int8_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray->retrieveInt8(), 1);
}

TEST_F(ByteArrayWriterTest, WriteInt64)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	{
		ByteArrayWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.iovec();
		EXPECT_EQ(writeVec.iovenLength(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, ByteArray::BufferNodeSize);
		auto value = static_cast<int64_t>(::htobe64(2));
		writeToIOVec(writeVec.iovec(), 0, &value, sizeof(int64_t));
		writeVec.adjustByteArray(sizeof(int64_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray->retrieveInt64(), 2);
}

TEST_F(ByteArrayWriterTest, WriteInt64WithLength)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	{
		ByteArrayWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.iovec();
		EXPECT_EQ(writeVec.iovenLength(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, ByteArray::BufferNodeSize);
		auto value = static_cast<int64_t>(::htobe64(2));
		writeToIOVec(writeVec.iovec(), 0, &value, sizeof(int64_t));
		writeVec.adjustByteArray(sizeof(int8_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray->retrieveInt64(), 0);
}

TEST_F(ByteArrayWriterTest, WriteNegativeInt64)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	{
		ByteArrayWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.iovec();
		EXPECT_EQ(writeVec.iovenLength(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, ByteArray::BufferNodeSize);
		auto value = static_cast<int64_t>(::htobe64(static_cast<uint64_t>(-2)));
		writeToIOVec(writeVec.iovec(), 0, &value, sizeof(int64_t));
		writeVec.adjustByteArray(sizeof(int64_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray->retrieveInt64(), -2);
}

TEST_F(ByteArrayWriterTest, WriteRaw)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	const char str[] = "test string";
	std::size_t stringLength = (std::strlen(str) + 1) * sizeof(char);
	{
		ByteArrayWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.iovec();
		EXPECT_EQ(writeVec.iovenLength(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, ByteArray::BufferNodeSize);
		writeToIOVec(writeVec.iovec(), 0, str, stringLength);
		writeVec.adjustByteArray(stringLength);
	}
	EXPECT_EQ(byteArray->readableBytes(), stringLength);
	char buffer[100];
	EXPECT_EQ(byteArray->retrieveRaw(buffer, stringLength), stringLength);
	EXPECT_STREQ(str, buffer);
}

TEST_F(ByteArrayWriterTest, WriteTwice)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	std::string str = "test string";
	std::size_t stringLength = str.length() * sizeof(char);
	{
		ByteArrayWriterWithLock writeVecFir(byteArray);
		::iovec *vecFir = writeVecFir.iovec();
		EXPECT_EQ(writeVecFir.iovenLength(), 1);
		EXPECT_NE(vecFir, nullptr);
		EXPECT_EQ(vecFir[0].iov_len, ByteArray::BufferNodeSize);
		writeToIOVec(vecFir, 0, str.data(), stringLength);
		writeVecFir.adjustByteArray(stringLength);
	}
	EXPECT_EQ(byteArray->readableBytes(), stringLength);
	EXPECT_EQ(byteArray->retrieveString(stringLength), str);

	str = "abcs string";
	{
		ByteArrayWriterWithLock writeVecSec(byteArray);
		::iovec *vecSec = writeVecSec.iovec();
		EXPECT_EQ(writeVecSec.iovenLength(), 1);
		EXPECT_NE(vecSec, nullptr);
		EXPECT_EQ(vecSec[0].iov_len, ByteArray::BufferNodeSize - stringLength);
		writeToIOVec(vecSec, 0, str.data(), stringLength);
		writeVecSec.adjustByteArray(stringLength);
	}
	EXPECT_EQ(byteArray->readableBytes(), stringLength);
	EXPECT_EQ(byteArray->retrieveString(stringLength), str);
}

TEST_F(ByteArrayWriterTest, WriteCrossNodeInt8)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	char raw[ByteArray::BufferNodeSize * 3] = {'\0'};
	byteArray->writeRaw(raw, ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize + 1);
	{
		ByteArrayWriterWithLock writeVec(byteArray);
		ASSERT_EQ(writeVec.iovenLength(), 2);
		EXPECT_NE(writeVec.iovec(), nullptr);
		int8_t data = 1;
		writeToIOVec(writeVec.iovec(), 0, &data, sizeof(int8_t));
		data = 2;
		writeToIOVec(writeVec.iovec(), 1, &data, sizeof(int8_t));
		writeVec.adjustByteArray(2);
	}
	EXPECT_EQ(byteArray->readableBytes(), ByteArray::BufferNodeSize * 3 + sizeof(int8_t));
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));

	EXPECT_EQ(byteArray->retrieveRaw(raw, ByteArray::BufferNodeSize * 3 - 1), ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t) * 2);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));

	EXPECT_EQ(byteArray->retrieveInt8(), 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));

	EXPECT_EQ(byteArray->retrieveInt8(), 2);
	EXPECT_EQ(byteArray->readableBytes(), 0);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));
}

TEST_F(ByteArrayWriterTest, WriteCrossNodeInt64)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	char raw[ByteArray::BufferNodeSize * 3] = {'\0'};
	byteArray->writeRaw(raw, ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize + 1);
	{
		ByteArrayWriterWithLock writeVec(byteArray);
		ASSERT_EQ(writeVec.iovenLength(), 2);
		EXPECT_NE(writeVec.iovec(), nullptr);
		auto data = static_cast<int64_t>(htobe64(9223372000004775807));
		writeToIOVec(writeVec.iovec(), 0, &data, sizeof(int8_t));
		writeToIOVec(writeVec.iovec(), 1, reinterpret_cast<char *>(&data) + sizeof(int8_t), sizeof(int8_t) * 7);
		writeVec.adjustByteArray(sizeof(int64_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), ByteArray::BufferNodeSize * 3 + sizeof(int8_t) * 7);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t) * 7);

	EXPECT_EQ(byteArray->retrieveRaw(raw, ByteArray::BufferNodeSize * 3 - 1), ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t) * 7);

	EXPECT_EQ(byteArray->retrieveInt64(), 9223372000004775807);
	EXPECT_EQ(byteArray->readableBytes(), 0);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t) * 7);
}
