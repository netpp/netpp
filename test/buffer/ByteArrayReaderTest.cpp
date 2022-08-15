#include <gtest/gtest.h>
#include "buffer/ByteArray.h"
#include "buffer/BufferIOConversion.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <sys/types.h>
#include <endian.h>
#include <sys/uio.h>
}

using namespace netpp;

class ByteArrayReaderTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(ByteArrayReaderTest, EmptyByteArray)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	{
		ByteArrayReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.iovenLength(), 0);
		EXPECT_EQ(readVec.iovec(), nullptr);
		readVec.adjustByteArray(0);
	}
	EXPECT_EQ(byteArray->readableBytes(), 0);
}

TEST_F(ByteArrayReaderTest, ReadInt8)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	byteArray->writeInt8(1);
	{
		ByteArrayReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.iovenLength(), 1);
		EXPECT_NE(readVec.iovec(), nullptr);
		EXPECT_EQ(readVec.iovec()[0].iov_len, sizeof(int8_t));
		readVec.adjustByteArray(sizeof(int8_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), 0);
}

TEST_F(ByteArrayReaderTest, ReadInt64)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	byteArray->writeInt64(2);
	{
		ByteArrayReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.iovenLength(), 1);
		EXPECT_NE(readVec.iovec(), nullptr);
		EXPECT_EQ(readVec.iovec()[0].iov_len, sizeof(int64_t));
		readVec.adjustByteArray(sizeof(int64_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), 0);
}

TEST_F(ByteArrayReaderTest, ReadInt8FromInt64)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	byteArray->writeInt64(2);
	{
		ByteArrayReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.iovenLength(), 1);
		EXPECT_NE(readVec.iovec(), nullptr);
		EXPECT_EQ(readVec.iovec()[0].iov_len, sizeof(int64_t));
		readVec.adjustByteArray(sizeof(int8_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int64_t) - sizeof(int8_t));
	EXPECT_EQ(byteArray->retrieveInt8(), 0);
	EXPECT_EQ(byteArray->retrieveInt8(), 0);
	EXPECT_EQ(byteArray->retrieveInt8(), 0);
	EXPECT_EQ(byteArray->retrieveInt32(), 2);
}

TEST_F(ByteArrayReaderTest, ReadString)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	std::string str = "test string";
	byteArray->writeString(str);
	{
		ByteArrayReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.iovenLength(), 1);
		EXPECT_NE(readVec.iovec(), nullptr);
		EXPECT_EQ(readVec.iovec()[0].iov_len, str.length() * sizeof(char));
		readVec.adjustByteArray(str.length() * sizeof(char));
	}
	EXPECT_EQ(byteArray->readableBytes(), 0);
}

TEST_F(ByteArrayReaderTest, ReadTwice)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	std::string str = "test string";
	std::size_t stringLength = str.length() * sizeof(char);
	byteArray->writeString(str);
	{
		ByteArrayReaderWithLock readVecFir(byteArray);
		::iovec *vecFir = readVecFir.iovec();
		EXPECT_EQ(readVecFir.iovenLength(), 1);
		EXPECT_NE(vecFir, nullptr);
		EXPECT_EQ(vecFir[0].iov_len, stringLength);
		readVecFir.adjustByteArray(str.length() * sizeof(char));
	}
	EXPECT_EQ(byteArray->readableBytes(), 0);

	str = "abcs string";
	byteArray->writeString(str);
	{
		ByteArrayReaderWithLock readVecSec(byteArray);
		::iovec *vecSec = readVecSec.iovec();
		EXPECT_EQ(readVecSec.iovenLength(), 1);
		EXPECT_NE(vecSec, nullptr);
		EXPECT_EQ(vecSec[0].iov_len, stringLength);
		readVecSec.adjustByteArray(stringLength);
	}
	EXPECT_EQ(byteArray->readableBytes(), 0);
}

TEST_F(ByteArrayReaderTest, ReadCrossNodeInt8)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	char raw[ByteArray::BufferNodeSize * 3] = {'\0'};
	byteArray->writeRaw(raw, ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize + 1);
	byteArray->writeInt8(1);
	byteArray->writeInt8(2);
	EXPECT_EQ(byteArray->readableBytes(), ByteArray::BufferNodeSize * 3 + sizeof(int8_t));
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));

	EXPECT_EQ(byteArray->retrieveRaw(raw, ByteArray::BufferNodeSize * 3 - 1), ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t) * 2);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));
	{
		ByteArrayReaderWithLock readVec(byteArray);
		ASSERT_EQ(readVec.iovenLength(), 2);
		EXPECT_NE(readVec.iovec(), nullptr);
		int8_t data;
		void *base = readVec.iovec()[0].iov_base;
		std::memcpy(&data, base, sizeof(int8_t));
		EXPECT_EQ(data, 1);
		base = readVec.iovec()[1].iov_base;
		std::memcpy(&data, base, sizeof(int8_t));
		EXPECT_EQ(data, 2);
		readVec.adjustByteArray(sizeof(int8_t) * 2);
	}
	EXPECT_EQ(byteArray->readableBytes(), 0);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize * 4 - sizeof(int8_t));
}

TEST_F(ByteArrayReaderTest, ReadCrossNodeInt64)
{
	std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
	char raw[ByteArray::BufferNodeSize * 3] = {'\0'};
	byteArray->writeRaw(raw, ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize + 1);
	byteArray->writeInt64(9223372000004775807);
	EXPECT_EQ(byteArray->readableBytes(), ByteArray::BufferNodeSize * 3 - 1 + sizeof(int64_t));
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t) * 7);

	EXPECT_EQ(byteArray->retrieveRaw(raw, ByteArray::BufferNodeSize * 3 - 1), ByteArray::BufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t) * 7);
	{
		ByteArrayReaderWithLock readVec(byteArray);
		ASSERT_EQ(readVec.iovenLength(), 2);
		EXPECT_NE(readVec.iovec(), nullptr);
		int64_t data;
		void *base = readVec.iovec()[0].iov_base;
		std::memcpy(&data, base, sizeof(int8_t));
		base = readVec.iovec()[1].iov_base;
		std::memcpy(reinterpret_cast<char *>(&data) + 1, base, sizeof(int8_t) * 7);
		EXPECT_EQ(be64toh(data), 9223372000004775807);
		readVec.adjustByteArray(sizeof(int64_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), 0);
	EXPECT_EQ(byteArray->writeableBytes(), ByteArray::BufferNodeSize * 4 - sizeof(int8_t) * 7);
}
