#include <gtest/gtest.h>
#include "ByteArray.h"
#include "socket/SocketIO.h"
#include <cstring>
extern "C" {
#include <sys/types.h>
#include <endian.h>
#include <sys/uio.h>
}

class ByteArrayTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
	// the value of ByteArray::BufferNode::BufferSize
	static constexpr int bufferNodeSize = 1024;

	void writeToIOVec(::iovec *vec, int count, const void *data, int length)
	{
		std::memcpy(vec->iov_base, data, length);
	}
};

TEST_F(ByteArrayTest, ReadWriteWithinNode)
{
	netpp::ByteArray byteArray;
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeInt8(1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray.retrieveInt8(), 1);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeInt16(2);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int16_t));
	EXPECT_EQ(byteArray.retrieveInt16(), 2);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeInt32(3);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int32_t));
	EXPECT_EQ(byteArray.retrieveInt32(), 3);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeInt64(4);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray.retrieveInt64(), 4);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeUInt8(5);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint8_t));
	EXPECT_EQ(byteArray.retrieveUInt8(), 5);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeUInt16(6);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint16_t));
	EXPECT_EQ(byteArray.retrieveUInt16(), 6);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeUInt32(7);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint32_t));
	EXPECT_EQ(byteArray.retrieveUInt32(), 7);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeUInt64(8);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint64_t));
	EXPECT_EQ(byteArray.retrieveUInt64(), 8);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeFloat(9.1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(float));
	EXPECT_FLOAT_EQ(byteArray.retrieveFloat(), 9.1);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeDouble(10.1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(double));
	EXPECT_FLOAT_EQ(byteArray.retrieveDouble(), 10.1);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	std::string testStr("11");
	byteArray.writeString(testStr);
	EXPECT_EQ(byteArray.readableBytes(), testStr.length() * sizeof(char));
	EXPECT_EQ(byteArray.retrieveString(testStr.length()), testStr);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeRaw("12", 3);
	char buffer[3];
	EXPECT_EQ(byteArray.readableBytes(), sizeof(char) * 3);
	EXPECT_EQ(byteArray.retrieveRaw(buffer, 3), sizeof(char) * 3);
	EXPECT_STREQ(buffer, "12");
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, ReadWriteCrossNode)
{
	netpp::ByteArray byteArray;
	char raw[bufferNodeSize] = {'\0'};
	byteArray.writeRaw(raw, bufferNodeSize - 1);
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize - 1);

	byteArray.writeInt8(1);
	byteArray.writeInt8(2);
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize + 1);
	byteArray.retrieveRaw(raw, bufferNodeSize - 1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t) * 2);
	EXPECT_EQ(byteArray.retrieveInt8(), 1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray.retrieveInt8(), 2);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeRaw(raw, bufferNodeSize);
	byteArray.writeInt8(3);
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize + sizeof(int8_t));
	byteArray.retrieveRaw(raw, bufferNodeSize);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray.retrieveInt8(), 3);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeRaw(raw, bufferNodeSize);
	byteArray.writeInt64(4);
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize + sizeof(int64_t));
	byteArray.retrieveRaw(raw, bufferNodeSize);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray.retrieveInt64(), 4);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeRaw(raw, bufferNodeSize / 2);
	byteArray.writeInt64(5);
	byteArray.writeRaw(raw, bufferNodeSize / 2);
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize + sizeof(int64_t));
	byteArray.retrieveRaw(raw, bufferNodeSize / 2);
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize / 2 + sizeof(int64_t));
	EXPECT_EQ(byteArray.retrieveInt64(), 5);
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize / 2);
	byteArray.retrieveRaw(raw, bufferNodeSize / 2);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, ReadWriteByteArrayContinuous)
{
	netpp::ByteArray byteArray;
	byteArray.writeString("abc");
	EXPECT_EQ(byteArray.readableBytes(), 3);
	byteArray.writeString("efgh");
	EXPECT_EQ(byteArray.readableBytes(), 7);
	byteArray.writeString("ijklmn");
	EXPECT_EQ(byteArray.readableBytes(), 13);
	EXPECT_EQ(byteArray.retrieveString(3), std::string("abc"));
	EXPECT_EQ(byteArray.retrieveString(4), std::string("efgh"));
	EXPECT_EQ(byteArray.retrieveString(6), std::string("ijklmn"));
}

TEST_F(ByteArrayTest, ByteArrayAsIOVecInNode)
{
	// empty
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		{
		netpp::internal::socket::ByteArrayIOVectorReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.count(), 0);
		EXPECT_EQ(readVec.vec(), nullptr);
		}
		{
		netpp::internal::socket::ByteArrayIOVectorWriterWithLock writeVec(byteArray);
		EXPECT_EQ(writeVec.count(), 1);
		EXPECT_NE(writeVec.vec(), nullptr);
		EXPECT_EQ(byteArray->readableBytes(), 0);
		}
	}
	// read int8_t
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		byteArray->writeInt8(1);
		{
		netpp::internal::socket::ByteArrayIOVectorReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.count(), 1);
		EXPECT_NE(readVec.vec(), nullptr);
		EXPECT_EQ(readVec.vec()[0].iov_len, sizeof(int8_t));
		readVec.adjustByteArray(sizeof(int8_t));
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);
	}
	// read int64_t
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		byteArray->writeInt64(2);
		{
		netpp::internal::socket::ByteArrayIOVectorReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.count(), 1);
		EXPECT_NE(readVec.vec(), nullptr);
		EXPECT_EQ(readVec.vec()[0].iov_len, sizeof(int64_t));
		readVec.adjustByteArray(sizeof(int64_t));
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);
	}
	// read string
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		std::string str = "test string";
		byteArray->writeString(str);
		{
		netpp::internal::socket::ByteArrayIOVectorReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.count(), 1);
		EXPECT_NE(readVec.vec(), nullptr);
		EXPECT_EQ(readVec.vec()[0].iov_len, str.length() * sizeof(char));
		readVec.adjustByteArray(str.length() * sizeof(char));
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);
	}
	// read raw
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		const char str[] = "test string";
		byteArray->writeRaw(str, std::strlen(str) + 1);
		{
		netpp::internal::socket::ByteArrayIOVectorReaderWithLock readVec(byteArray);
		EXPECT_EQ(readVec.count(), 1);
		EXPECT_NE(readVec.vec(), nullptr);
		EXPECT_EQ(readVec.vec()[0].iov_len, (std::strlen(str) + 1) * sizeof(char));
		readVec.adjustByteArray((std::strlen(str) + 1) * sizeof(char));
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);
	}
	// write int8_t
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		{
		netpp::internal::socket::ByteArrayIOVectorWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.vec();
		EXPECT_EQ(writeVec.count(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, bufferNodeSize);
		int8_t value = 1;
		writeToIOVec(writeVec.vec(), writeVec.count(), &value, sizeof(int8_t));
		writeVec.adjustByteArray(sizeof(int8_t));
		}
		EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t));
		EXPECT_EQ(byteArray->retrieveInt8(), 1);
	}
	// write int64_t
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		{
		netpp::internal::socket::ByteArrayIOVectorWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.vec();
		EXPECT_EQ(writeVec.count(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, bufferNodeSize);
		int64_t value = ::htobe64(2);
		writeToIOVec(writeVec.vec(), writeVec.count(), &value, sizeof(int64_t));
		writeVec.adjustByteArray(sizeof(int64_t));
		}
		EXPECT_EQ(byteArray->readableBytes(), sizeof(int64_t));
		EXPECT_EQ(byteArray->retrieveInt64(), 2);
	}
	// write string
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		std::string str = "test string";
		std::size_t stringLength = (str.length()) * sizeof(char);
		{
		netpp::internal::socket::ByteArrayIOVectorWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.vec();
		EXPECT_EQ(writeVec.count(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, bufferNodeSize);
		writeToIOVec(writeVec.vec(), writeVec.count(), str.data(), stringLength);
		writeVec.adjustByteArray(stringLength);
		}
		EXPECT_EQ(byteArray->readableBytes(), stringLength);
		EXPECT_EQ(byteArray->retrieveString(stringLength), str);
	}
	// write raw
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		const char str[] = "test string";
		std::size_t stringLength = (std::strlen(str) + 1) * sizeof(char);
		{
		netpp::internal::socket::ByteArrayIOVectorWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.vec();
		EXPECT_EQ(writeVec.count(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, bufferNodeSize);
		writeToIOVec(writeVec.vec(), writeVec.count(), str, stringLength);
		writeVec.adjustByteArray(stringLength);
		}
		EXPECT_EQ(byteArray->readableBytes(), stringLength);
		char buffer[100];
		EXPECT_EQ(byteArray->retrieveRaw(buffer, stringLength), stringLength);
		EXPECT_STREQ(str, buffer);
	}
	// write twice
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		std::string str = "test string";
		std::size_t stringLength = str.length() * sizeof(char);
		{
		netpp::internal::socket::ByteArrayIOVectorWriterWithLock writeVecFir(byteArray);
		::iovec *vecFir = writeVecFir.vec();
		EXPECT_EQ(writeVecFir.count(), 1);
		EXPECT_NE(vecFir, nullptr);
		EXPECT_EQ(vecFir[0].iov_len, bufferNodeSize);
		writeToIOVec(writeVecFir.vec(), writeVecFir.count(), str.data(), stringLength);
		writeVecFir.adjustByteArray(stringLength);
		}
		EXPECT_EQ(byteArray->readableBytes(), stringLength);
		EXPECT_EQ(byteArray->retrieveString(stringLength), str);

		str = "abcs string";
		{
		netpp::internal::socket::ByteArrayIOVectorWriterWithLock writeVecSec(byteArray);
		::iovec *vecSec = writeVecSec.vec();
		EXPECT_EQ(writeVecSec.count(), 1);
		EXPECT_NE(vecSec, nullptr);
		EXPECT_EQ(vecSec[0].iov_len, bufferNodeSize - stringLength);
		writeToIOVec(writeVecSec.vec(), writeVecSec.count(), str.data(), stringLength);
		writeVecSec.adjustByteArray(stringLength);
		}
		EXPECT_EQ(byteArray->readableBytes(), stringLength);
		EXPECT_EQ(byteArray->retrieveString(stringLength), str);
	}
	// read twice
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		std::string str = "test string";
		std::size_t stringLength = str.length() * sizeof(char);
		byteArray->writeString(str);
		{
		netpp::internal::socket::ByteArrayIOVectorReaderWithLock readVecFir(byteArray);
		::iovec *vecFir = readVecFir.vec();
		EXPECT_EQ(readVecFir.count(), 1);
		EXPECT_NE(vecFir, nullptr);
		EXPECT_EQ(vecFir[0].iov_len, stringLength);
		readVecFir.adjustByteArray(str.length() * sizeof(char));
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);

		str = "abcs string";
		byteArray->writeString(str);
		{
		netpp::internal::socket::ByteArrayIOVectorReaderWithLock readVecSec(byteArray);
		::iovec *vecSec = readVecSec.vec();
		EXPECT_EQ(readVecSec.count(), 1);
		EXPECT_NE(vecSec, nullptr);
		EXPECT_EQ(vecSec[0].iov_len, stringLength);
		readVecSec.adjustByteArray(stringLength);
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);
	}
	// none empty write
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		std::string str = "test";
		byteArray->writeString(str);
		std::size_t strLength = str.length() * sizeof(char);
		byteArray->retrieveString(strLength);
		{
		netpp::internal::socket::ByteArrayIOVectorWriterWithLock writeVec(byteArray);
		::iovec *vec = writeVec.vec();
		EXPECT_EQ(writeVec.count(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, bufferNodeSize - strLength);
		str = "abcd";
		writeToIOVec(writeVec.vec(), writeVec.count(), str.data(), strLength);
		writeVec.adjustByteArray(strLength);
		std::string vecStr;
		vecStr.resize(strLength);
		std::memcpy(vecStr.data(), vec[0].iov_base, strLength);
		EXPECT_EQ(vecStr, str);
		}
	}
	// none empty read
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		std::string str = "test";
		byteArray->writeString(str);
		std::size_t strLength = str.length() * sizeof(char);
		byteArray->retrieveString(strLength);
		str = "abcd";
		byteArray->writeString(str);
		{
		netpp::internal::socket::ByteArrayIOVectorReaderWithLock readVec(byteArray);
		::iovec *vec = readVec.vec();
		EXPECT_EQ(readVec.count(), 1);
		EXPECT_NE(vec, nullptr);
		EXPECT_EQ(vec[0].iov_len, strLength);
		std::string vecStr;
		vecStr.resize(strLength);
		std::memcpy(vecStr.data(), vec[0].iov_base, strLength);
		EXPECT_EQ(vecStr, str);
		}
	}
	// TODO: test adjustByteArray with arg -1
	// writeVec.adjustByteArray(-1);
}

TEST_F(ByteArrayTest, ByteArrayAsIOVecCrossNode)
{
}
