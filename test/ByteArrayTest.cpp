#include <gtest/gtest.h>
#include "ByteArray.h"
#include "internal/socket/SocketIO.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <sys/types.h>
#include <endian.h>
#include <sys/uio.h>
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"

class ByteArrayTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
	// the value of ByteArray::BufferNode::BufferNodeSize
	static constexpr int bufferNodeSize = 1024;

	void writeToIOVec(::iovec *vec, int index, const void *data, int length)
	{
		std::memcpy(vec[index].iov_base, data, length);
	}
};

TEST_F(ByteArrayTest, WithinNodeReadWrite)
{
	netpp::ByteArray byteArray;
	int writeBytes = 0;
	int writeInTotal = 0;
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);

	byteArray.writeInt8(1);
	writeBytes = sizeof(int8_t);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeBytes);
	EXPECT_EQ(byteArray.retrieveInt8(), 1);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	byteArray.writeInt16(2);
	writeBytes = sizeof(int16_t);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_EQ(byteArray.retrieveInt16(), 2);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	byteArray.writeInt32(3);
	writeBytes = sizeof(int32_t);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_EQ(byteArray.retrieveInt32(), 3);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	byteArray.writeInt64(4);
	writeBytes = sizeof(int64_t);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_EQ(byteArray.retrieveInt64(), 4);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	byteArray.writeUInt8(5);
	writeBytes = sizeof(uint8_t);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_EQ(byteArray.retrieveUInt8(), 5);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	byteArray.writeUInt16(6);
	writeBytes = sizeof(uint16_t);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_EQ(byteArray.retrieveUInt16(), 6);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	byteArray.writeUInt32(7);
	writeBytes = sizeof(uint32_t);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_EQ(byteArray.retrieveUInt32(), 7);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	byteArray.writeUInt64(8);
	writeBytes = sizeof(uint64_t);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_EQ(byteArray.retrieveUInt64(), 8);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	byteArray.writeFloat(9.1);
	writeBytes = sizeof(float);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_FLOAT_EQ(byteArray.retrieveFloat(), 9.1);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	byteArray.writeDouble(10.1);
	writeBytes = sizeof(double);
	writeInTotal += writeBytes;
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_FLOAT_EQ(byteArray.retrieveDouble(), 10.1);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	std::string testStr("11");
	writeBytes = testStr.length() * sizeof(char);
	writeInTotal += writeBytes;
	byteArray.writeString(testStr);
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_EQ(byteArray.retrieveString(testStr.length()), testStr);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);

	writeBytes = sizeof(char) * 3;
	writeInTotal += writeBytes;
	byteArray.writeRaw("12", 3);
	char buffer[3];
	EXPECT_EQ(byteArray.readableBytes(), writeBytes);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
	EXPECT_EQ(byteArray.retrieveRaw(buffer, 3), 3);
	EXPECT_STREQ(buffer, "12");
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - writeInTotal);
}

TEST_F(ByteArrayTest, WithInNodeReadFromEmpty)
{
	netpp::ByteArray byteArray;
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);

	char raw[bufferNodeSize];
	EXPECT_EQ(byteArray.retrieveRaw(raw, bufferNodeSize), 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);

	byteArray.retrieveString(bufferNodeSize);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);

	byteArray.retrieveDouble();
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);
}

TEST_F(ByteArrayTest, CrossNodeMoveHead)
{
	netpp::ByteArray byteArray;
	char raw[bufferNodeSize] = {'\0'};
	byteArray.writeRaw(raw, bufferNodeSize);	// alloc new node
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);

	EXPECT_EQ(byteArray.retrieveRaw(raw, bufferNodeSize), bufferNodeSize);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);

	byteArray.writeRaw(raw, bufferNodeSize);	// head node will move to tail
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);

	EXPECT_EQ(byteArray.retrieveRaw(raw, bufferNodeSize), bufferNodeSize);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);

	byteArray.writeInt8(1);
	EXPECT_EQ(byteArray.readableBytes(), 1);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - 1);

	EXPECT_EQ(byteArray.retrieveInt8(), 1);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - 1);
}

TEST_F(ByteArrayTest, CrossNodeReadWriteBigEntry)
{
	netpp::ByteArray byteArray;
	char raw[bufferNodeSize * 10] = {'\0'};
	byteArray.writeRaw(raw, bufferNodeSize * 10);	// 16 nodes, last 6 is usable
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize * 10);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize * 6);
	EXPECT_EQ(byteArray.retrieveRaw(raw, bufferNodeSize * 10), bufferNodeSize * 10);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize * 6);

	byteArray.writeRaw(raw, bufferNodeSize * 6);	// move head, 10 node is usable
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize * 6);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize * 10);
	EXPECT_EQ(byteArray.retrieveRaw(raw, bufferNodeSize * 6), bufferNodeSize * 6);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize * 10);
}

TEST_F(ByteArrayTest, CrossNodeReadWriteInt8)
{
	netpp::ByteArray byteArray;
	char raw[bufferNodeSize] = {'\0'};
	byteArray.writeRaw(raw, bufferNodeSize - 1);	// write 1023 bytes
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize - 1);
	EXPECT_EQ(byteArray.writeableBytes(), 1);

	byteArray.writeInt8(1);		// write 1 byte, new node alloc
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize);
	byteArray.writeInt8(2);		// write 1 byte
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize + 1);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - 1);

	EXPECT_EQ(byteArray.retrieveRaw(raw, bufferNodeSize - 1), bufferNodeSize - 1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t) * 2);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - 1);

	EXPECT_EQ(byteArray.retrieveInt8(), 1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - 1);

	EXPECT_EQ(byteArray.retrieveInt8(), 2);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - 1);
}

TEST_F(ByteArrayTest, CrossNodeReadWriteInt64)
{
	netpp::ByteArray byteArray;
	char raw[bufferNodeSize] = {'\0'};
	byteArray.writeRaw(raw, bufferNodeSize - 1);	// write 1023 bytes
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize - 1);
	EXPECT_EQ(byteArray.writeableBytes(), 1);

	byteArray.writeInt64(9223372000004775807);		// new node alloc
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - sizeof(int64_t) + 1);
	EXPECT_EQ(byteArray.readableBytes(), bufferNodeSize + sizeof(int64_t) - 1);

	EXPECT_EQ(byteArray.retrieveRaw(raw, bufferNodeSize - 1), bufferNodeSize - 1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - sizeof(int64_t) + 1);

	EXPECT_EQ(byteArray.retrieveInt64(), 9223372000004775807);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), bufferNodeSize - sizeof(int64_t) + 1);
}

TEST_F(ByteArrayTest, ContinuousReadWriteByteArray)
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

TEST_F(ByteArrayTest, WithinNodeByteArrayAsIOVec)
{
	// empty
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		{
			netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
			EXPECT_EQ(readVec.iovenLength(), 0);
			EXPECT_EQ(readVec.iovec(), nullptr);
		}
		{
			netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
			EXPECT_EQ(writeVec.iovenLength(), 1);
			EXPECT_NE(writeVec.iovec(), nullptr);
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);
	}
	// read int8_t
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		byteArray->writeInt8(1);
		{
			netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
			EXPECT_EQ(readVec.iovenLength(), 1);
			EXPECT_NE(readVec.iovec(), nullptr);
			EXPECT_EQ(readVec.iovec()[0].iov_len, sizeof(int8_t));
			readVec.adjustByteArray(sizeof(int8_t));
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);
	}
	// read int64_t
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		byteArray->writeInt64(2);
		{
			netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
			EXPECT_EQ(readVec.iovenLength(), 1);
			EXPECT_NE(readVec.iovec(), nullptr);
			EXPECT_EQ(readVec.iovec()[0].iov_len, sizeof(int64_t));
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
			netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
			EXPECT_EQ(readVec.iovenLength(), 1);
			EXPECT_NE(readVec.iovec(), nullptr);
			EXPECT_EQ(readVec.iovec()[0].iov_len, str.length() * sizeof(char));
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
			netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
			EXPECT_EQ(readVec.iovenLength(), 1);
			EXPECT_NE(readVec.iovec(), nullptr);
			EXPECT_EQ(readVec.iovec()[0].iov_len, (std::strlen(str) + 1) * sizeof(char));
			readVec.adjustByteArray((std::strlen(str) + 1) * sizeof(char));
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);
	}
	// write int8_t
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		{
			netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
			::iovec *vec = writeVec.iovec();
			EXPECT_EQ(writeVec.iovenLength(), 1);
			EXPECT_NE(vec, nullptr);
			EXPECT_EQ(vec[0].iov_len, bufferNodeSize);
			int8_t value = 1;
			writeToIOVec(writeVec.iovec(), 0, &value, sizeof(int8_t));
			writeVec.adjustByteArray(sizeof(int8_t));
		}
		EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t));
		EXPECT_EQ(byteArray->retrieveInt8(), 1);
	}
	// write int64_t
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		{
			netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
			::iovec *vec = writeVec.iovec();
			EXPECT_EQ(writeVec.iovenLength(), 1);
			EXPECT_NE(vec, nullptr);
			EXPECT_EQ(vec[0].iov_len, bufferNodeSize);
			int64_t value = ::htobe64(2);
			writeToIOVec(writeVec.iovec(), 0, &value, sizeof(int64_t));
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
			netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
			::iovec *vec = writeVec.iovec();
			EXPECT_EQ(writeVec.iovenLength(), 1);
			EXPECT_NE(vec, nullptr);
			EXPECT_EQ(vec[0].iov_len, bufferNodeSize);
			writeToIOVec(writeVec.iovec(), 0, str.data(), stringLength);
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
			netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
			::iovec *vec = writeVec.iovec();
			EXPECT_EQ(writeVec.iovenLength(), 1);
			EXPECT_NE(vec, nullptr);
			EXPECT_EQ(vec[0].iov_len, bufferNodeSize);
			writeToIOVec(writeVec.iovec(), 0, str, stringLength);
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
			netpp::internal::socket::ByteArrayWriterWithLock writeVecFir(byteArray);
			::iovec *vecFir = writeVecFir.iovec();
			EXPECT_EQ(writeVecFir.iovenLength(), 1);
			EXPECT_NE(vecFir, nullptr);
			EXPECT_EQ(vecFir[0].iov_len, bufferNodeSize);
			writeToIOVec(vecFir, 0, str.data(), stringLength);
			writeVecFir.adjustByteArray(stringLength);
		}
		EXPECT_EQ(byteArray->readableBytes(), stringLength);
		EXPECT_EQ(byteArray->retrieveString(stringLength), str);

		str = "abcs string";
		{
			netpp::internal::socket::ByteArrayWriterWithLock writeVecSec(byteArray);
			::iovec *vecSec = writeVecSec.iovec();
			EXPECT_EQ(writeVecSec.iovenLength(), 1);
			EXPECT_NE(vecSec, nullptr);
			EXPECT_EQ(vecSec[0].iov_len, bufferNodeSize - stringLength);
			writeToIOVec(vecSec, 0, str.data(), stringLength);
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
			netpp::internal::socket::ByteArrayReaderWithLock readVecFir(byteArray);
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
			netpp::internal::socket::ByteArrayReaderWithLock readVecSec(byteArray);
			::iovec *vecSec = readVecSec.iovec();
			EXPECT_EQ(readVecSec.iovenLength(), 1);
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
			netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
			::iovec *vec = writeVec.iovec();
			EXPECT_EQ(writeVec.iovenLength(), 1);
			EXPECT_NE(vec, nullptr);
			EXPECT_EQ(vec[0].iov_len, bufferNodeSize - strLength);
			str = "abcd";
			writeToIOVec(writeVec.iovec(), 0, str.data(), strLength);
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
			netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
			::iovec *vec = readVec.iovec();
			EXPECT_EQ(readVec.iovenLength(), 1);
			EXPECT_NE(vec, nullptr);
			EXPECT_EQ(vec[0].iov_len, strLength);
			std::string vecStr;
			vecStr.resize(strLength);
			std::memcpy(vecStr.data(), vec[0].iov_base, strLength);
			EXPECT_EQ(vecStr, str);
		}
	}
	// adjustByteArray with 0
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		{
			netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
			EXPECT_EQ(readVec.iovenLength(), 0);
			EXPECT_EQ(readVec.iovec(), nullptr);
			readVec.adjustByteArray(0);
		}
		{
			netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
			EXPECT_EQ(writeVec.iovenLength(), 1);
			EXPECT_NE(writeVec.iovec(), nullptr);
			writeVec.adjustByteArray(0);
		}
		EXPECT_EQ(byteArray->readableBytes(), 0);
	}
}

TEST_F(ByteArrayTest, CrossNodeByteArrayAsIOVecAdjustLength)
{
	// read cross node, just make sure length correct
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		char raw[bufferNodeSize] = {'\0'};
		byteArray->writeRaw(raw, bufferNodeSize - 1);
		byteArray->writeRaw(raw, 10);
		EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize - 1 + 10);
		{
			netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
			EXPECT_EQ(readVec.iovenLength(), 2);
			EXPECT_NE(readVec.iovec(), nullptr);
			EXPECT_EQ(readVec.iovec()[0].iov_len, bufferNodeSize);
			EXPECT_EQ(readVec.iovec()[1].iov_len, 9);
			readVec.adjustByteArray(13);
		}
		EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize - 1 + 10 - 13);
	}
	// write cross node, just make sure length correct
	{
		std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
		{
			netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
			writeVec.adjustByteArray(bufferNodeSize - 1 + 10);
		}
		EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize - 1 + 10);
	}
}

TEST_F(ByteArrayTest, CrossNodeByteArrayAsIOVecWriteInt8)
{
	std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
	char raw[bufferNodeSize * 3] = {'\0'};
	byteArray->writeRaw(raw, bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize + 1);
	{
		netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
		ASSERT_EQ(writeVec.iovenLength(), 2);
		EXPECT_NE(writeVec.iovec(), nullptr);
		int8_t data = 1;
		writeToIOVec(writeVec.iovec(), 0, &data, sizeof(int8_t));
		data = 2;
		writeToIOVec(writeVec.iovec(), 1, &data, sizeof(int8_t));
		writeVec.adjustByteArray(2);
	}
	EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize * 3 + sizeof(int8_t));
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t));

	EXPECT_EQ(byteArray->retrieveRaw(raw, bufferNodeSize * 3 - 1), bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t) * 2);
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t));

	EXPECT_EQ(byteArray->retrieveInt8(), 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t));

	EXPECT_EQ(byteArray->retrieveInt8(), 2);
	EXPECT_EQ(byteArray->readableBytes(), 0);
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t));
}

TEST_F(ByteArrayTest, CrossNodeByteArrayAsIOVecWriteInt64)
{
	std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
	char raw[bufferNodeSize * 3] = {'\0'};
	byteArray->writeRaw(raw, bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize + 1);
	{
		netpp::internal::socket::ByteArrayWriterWithLock writeVec(byteArray);
		ASSERT_EQ(writeVec.iovenLength(), 2);
		EXPECT_NE(writeVec.iovec(), nullptr);
		int64_t data = htobe64(9223372000004775807);
		writeToIOVec(writeVec.iovec(), 0, &data, sizeof(int8_t));
		writeToIOVec(writeVec.iovec(), 1, reinterpret_cast<char *>(&data) + sizeof(int8_t), sizeof(int8_t) * 7);
		writeVec.adjustByteArray(sizeof(int64_t));
	}
	EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize * 3 + sizeof(int8_t) * 7);
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t) * 7);

	EXPECT_EQ(byteArray->retrieveRaw(raw, bufferNodeSize * 3 - 1), bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t) * 7);

	EXPECT_EQ(byteArray->retrieveInt64(), 9223372000004775807);
	EXPECT_EQ(byteArray->readableBytes(), 0);
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t) * 7);
}

TEST_F(ByteArrayTest, CrossNodeByteArrayAsIOVecReadInt8)
{
	std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
	char raw[bufferNodeSize * 3] = {'\0'};
	byteArray->writeRaw(raw, bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize + 1);
	byteArray->writeInt8(1);
	byteArray->writeInt8(2);
	EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize * 3 + sizeof(int8_t));
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t));

	EXPECT_EQ(byteArray->retrieveRaw(raw, bufferNodeSize * 3 - 1), bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int8_t) * 2);
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t));
	{
		netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
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
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize * 4 - sizeof(int8_t));
}

TEST_F(ByteArrayTest, CrossNodeByteArrayAsIOVecReadInt64)
{
	std::shared_ptr<netpp::ByteArray> byteArray = std::make_shared<netpp::ByteArray>();
	char raw[bufferNodeSize * 3] = {'\0'};
	byteArray->writeRaw(raw, bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize + 1);
	byteArray->writeInt64(9223372000004775807);
	EXPECT_EQ(byteArray->readableBytes(), bufferNodeSize * 3 - 1 + sizeof(int64_t));
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t) * 7);

	EXPECT_EQ(byteArray->retrieveRaw(raw, bufferNodeSize * 3 - 1), bufferNodeSize * 3 - 1);
	EXPECT_EQ(byteArray->readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize - sizeof(int8_t) * 7);
	{
		netpp::internal::socket::ByteArrayReaderWithLock readVec(byteArray);
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
	EXPECT_EQ(byteArray->writeableBytes(), bufferNodeSize * 4 - sizeof(int8_t) * 7);
}

#pragma GCC diagnostic pop
