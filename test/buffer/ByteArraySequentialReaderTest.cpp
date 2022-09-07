#include <gtest/gtest.h>
#include "buffer/ByteArray.h"
#include "buffer/BufferIOConversion.h"
#include "ByteArrayGatherGetter.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <sys/types.h>
#include <endian.h>
#include <sys/uio.h>
}

using namespace netpp;

class ByteArraySequentialReaderTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(ByteArraySequentialReaderTest, SomeEmptyByteArray)
{
	static constexpr int byteArrayCount = 4;
	int fillByteArray = (1 << byteArrayCount) - 1;
	for (int i = 0; i <= fillByteArray; ++i)
	{
		std::vector<std::shared_ptr<ByteArray>> byteArrays;
		int byteArrayHasValue = 0;
		for (int j = 0; j < byteArrayCount; ++j)
		{
			std::shared_ptr<ByteArray> byteArray = std::make_shared<ByteArray>();
			if (i & j)
			{
				byteArray->writeInt8(2);
				++byteArrayHasValue;
			}
			byteArrays.emplace_back(std::move(byteArray));
		}
		{
			std::vector<std::shared_ptr<ByteArray>> tmp = byteArrays;
			SequentialByteArrayReaderWithLock readVec(std::move(tmp));
			EXPECT_EQ(get_iovec_len(readVec), byteArrayHasValue);
			for (auto j = 0; j < byteArrayHasValue; ++j)
			{
				EXPECT_EQ(get_iovec(readVec)[j].iov_len, sizeof(int8_t));
				int8_t data;
				std::memcpy(&data, get_iovec(readVec)[j].iov_base, sizeof(int8_t));
				EXPECT_EQ(data, 2);
				readVec.adjustByteArray(sizeof(int8_t));
			}
		}
		for (std::vector<std::shared_ptr<ByteArray>>::size_type j = 0; j < byteArrayCount; ++j)
			EXPECT_EQ(byteArrays[j]->readableBytes(), 0);
	}
}

TEST_F(ByteArraySequentialReaderTest, ReadInt64)
{
	std::shared_ptr<ByteArray> byteArray1 = std::make_shared<ByteArray>();
	std::shared_ptr<ByteArray> byteArray2 = std::make_shared<ByteArray>();
	byteArray1->writeInt64(2);
	byteArray2->writeInt64(3);
	{
		SequentialByteArrayReaderWithLock readVec({byteArray1, byteArray2});
		EXPECT_EQ(get_iovec_len(readVec), 2);
		EXPECT_NE(get_iovec(readVec), nullptr);
		EXPECT_EQ(get_iovec(readVec)[0].iov_len, sizeof(int64_t));
		EXPECT_EQ(get_iovec(readVec)[1].iov_len, sizeof(int64_t));
		readVec.adjustByteArray(sizeof(int64_t) * 2);
	}
	EXPECT_EQ(byteArray1->readableBytes(), 0);
	EXPECT_EQ(byteArray2->readableBytes(), 0);
}

TEST_F(ByteArraySequentialReaderTest, ReadInt64FromTwoInt64)
{
	std::shared_ptr<ByteArray> byteArray1 = std::make_shared<ByteArray>();
	std::shared_ptr<ByteArray> byteArray2 = std::make_shared<ByteArray>();
	byteArray1->writeInt64(2);
	byteArray2->writeInt64(3);
	{
		SequentialByteArrayReaderWithLock readVec({byteArray1, byteArray2});
		EXPECT_EQ(get_iovec_len(readVec), 2);
		EXPECT_NE(get_iovec(readVec), nullptr);
		EXPECT_EQ(get_iovec(readVec)[0].iov_len, sizeof(int64_t));
		EXPECT_EQ(get_iovec(readVec)[1].iov_len, sizeof(int64_t));
		readVec.adjustByteArray(sizeof(int64_t));
	}
	EXPECT_EQ(byteArray1->readableBytes(), 0);
	EXPECT_EQ(byteArray2->readableBytes(), sizeof(int64_t));
}

TEST_F(ByteArraySequentialReaderTest, ReadInt8FromInt64)
{
	std::shared_ptr<ByteArray> byteArray1 = std::make_shared<ByteArray>();
	std::shared_ptr<ByteArray> byteArray2 = std::make_shared<ByteArray>();
	byteArray1->writeInt64(2);
	byteArray2->writeInt64(3);
	{
		SequentialByteArrayReaderWithLock readVec({byteArray1, byteArray2});
		EXPECT_EQ(get_iovec_len(readVec), 2);
		EXPECT_NE(get_iovec(readVec), nullptr);
		EXPECT_EQ(get_iovec(readVec)[0].iov_len, sizeof(int64_t));
		EXPECT_EQ(get_iovec(readVec)[1].iov_len, sizeof(int64_t));
		readVec.adjustByteArray(sizeof(int8_t));
	}
	EXPECT_EQ(byteArray1->readableBytes(), sizeof(int64_t) - sizeof(int8_t));
	EXPECT_EQ(byteArray2->readableBytes(), sizeof(int64_t));

	EXPECT_EQ(byteArray1->retrieveInt8(), 0);
	EXPECT_EQ(byteArray1->retrieveInt8(), 0);
	EXPECT_EQ(byteArray1->retrieveInt8(), 0);
	EXPECT_EQ(byteArray1->retrieveInt32(), 2);
}

TEST_F(ByteArraySequentialReaderTest, ReadTwice)
{
	std::shared_ptr<ByteArray> byteArray1 = std::make_shared<ByteArray>();
	std::shared_ptr<ByteArray> byteArray2 = std::make_shared<ByteArray>();
	std::string b1Str = "test string";
	std::size_t b1Len = b1Str.length() * sizeof(char);
	std::string b2Str = "t string";
	std::size_t b2Len = b2Str.length() * sizeof(char);
	byteArray1->writeString(b1Str);
	byteArray2->writeString(b2Str);
	{
		SequentialByteArrayReaderWithLock readVecFir({byteArray1, byteArray2});
		::iovec *vecFir = get_iovec(readVecFir);
		EXPECT_EQ(get_iovec_len(readVecFir), 2);
		EXPECT_NE(vecFir, nullptr);
		EXPECT_EQ(vecFir[0].iov_len, b1Len);
		EXPECT_EQ(vecFir[1].iov_len, b2Len);

		std::string vec0Str;
		vec0Str.resize(b1Len);
		std::memcpy(vec0Str.data(), vecFir[0].iov_base, b1Len);
		EXPECT_EQ(vec0Str, b1Str);

		std::string vec1Str;
		vec1Str.resize(b2Len);
		std::memcpy(vec1Str.data(), vecFir[1].iov_base, b2Len);
		EXPECT_EQ(vec1Str, b2Str);

		readVecFir.adjustByteArray(b1Len + b2Len);
	}
	EXPECT_EQ(byteArray1->readableBytes(), 0);
	EXPECT_EQ(byteArray2->readableBytes(), 0);

	std::string b1Str1 = "ab string";
	std::size_t b1Len1 = b1Str1.length() * sizeof(char);
	std::string b2Str1 = "abc string";
	std::size_t b2Len1 = b2Str1.length() * sizeof(char);
	byteArray1->writeString(b1Str1);
	byteArray2->writeString(b2Str1);
	{
		SequentialByteArrayReaderWithLock readVecSec({byteArray1, byteArray2});
		::iovec *vecSec = get_iovec(readVecSec);
		EXPECT_EQ(get_iovec_len(readVecSec), 2);
		EXPECT_NE(vecSec, nullptr);
		EXPECT_EQ(vecSec[0].iov_len, b1Len1);
		EXPECT_EQ(vecSec[1].iov_len, b2Len1);

		std::string vec0Str;
		vec0Str.resize(b1Len1);
		std::memcpy(vec0Str.data(), vecSec[0].iov_base, b1Len1);
		EXPECT_EQ(vec0Str, b1Str1);

		std::string vec1Str;
		vec1Str.resize(b2Len1);
		std::memcpy(vec1Str.data(), vecSec[1].iov_base, b2Len1);
		EXPECT_EQ(vec1Str, b2Str1);

		readVecSec.adjustByteArray(b1Len1 + b2Len1);
	}
	EXPECT_EQ(byteArray1->readableBytes(), 0);
	EXPECT_EQ(byteArray2->readableBytes(), 0);
}

TEST_F(ByteArraySequentialReaderTest, CrossNodeReadInt8)
{
	std::shared_ptr<ByteArray> byteArray1 = std::make_shared<ByteArray>();
	std::shared_ptr<ByteArray> byteArray2 = std::make_shared<ByteArray>();
	auto fillByteArray = [](std::shared_ptr<ByteArray> &array){
		char raw[ByteArray::BufferNodeSize * 3] = {'\0'};
		array->writeRaw(raw, ByteArray::BufferNodeSize * 3 - 1);
		EXPECT_EQ(array->readableBytes(), ByteArray::BufferNodeSize * 3 - 1);
		EXPECT_EQ(array->writeableBytes(), ByteArray::BufferNodeSize + 1);
		static int8_t fillValue = 1;
		array->writeInt8(fillValue);
		++fillValue;
		array->writeInt8(fillValue);
		++fillValue;
		EXPECT_EQ(array->readableBytes(), ByteArray::BufferNodeSize * 3 + sizeof(int8_t));
		EXPECT_EQ(array->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));

		EXPECT_EQ(array->retrieveRaw(raw, ByteArray::BufferNodeSize * 3 - 1), ByteArray::BufferNodeSize * 3 - 1);
		EXPECT_EQ(array->readableBytes(), sizeof(int8_t) * 2);
		EXPECT_EQ(array->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));
	};
	fillByteArray(byteArray1);
	fillByteArray(byteArray2);
	{
		SequentialByteArrayReaderWithLock readVec({byteArray1, byteArray2});
		ASSERT_EQ(get_iovec_len(readVec), 4);
		EXPECT_NE(get_iovec(readVec), nullptr);
		int8_t data;
		std::memcpy(&data, get_iovec(readVec)[0].iov_base, sizeof(int8_t));
		EXPECT_EQ(data, 1);
		std::memcpy(&data, get_iovec(readVec)[1].iov_base, sizeof(int8_t));
		EXPECT_EQ(data, 2);
		std::memcpy(&data, get_iovec(readVec)[2].iov_base, sizeof(int8_t));
		EXPECT_EQ(data, 3);
		std::memcpy(&data, get_iovec(readVec)[3].iov_base, sizeof(int8_t));
		EXPECT_EQ(data, 4);
		readVec.adjustByteArray(sizeof(int8_t) * 4);
	}
	EXPECT_EQ(byteArray1->readableBytes(), 0);
	EXPECT_EQ(byteArray1->writeableBytes(), ByteArray::BufferNodeSize * 4 - sizeof(int8_t));
	EXPECT_EQ(byteArray2->readableBytes(), 0);
	EXPECT_EQ(byteArray2->writeableBytes(), ByteArray::BufferNodeSize * 4 - sizeof(int8_t));
}

TEST_F(ByteArraySequentialReaderTest, CrossNodeReadInt64)
{
	std::shared_ptr<ByteArray> byteArray1 = std::make_shared<ByteArray>();
	std::shared_ptr<ByteArray> byteArray2 = std::make_shared<ByteArray>();
	auto fillByteArray = [](std::shared_ptr<ByteArray> &array) {
		char raw[ByteArray::BufferNodeSize * 3] = {'\0'};
		array->writeRaw(raw, ByteArray::BufferNodeSize * 3 - 1);
		EXPECT_EQ(array->readableBytes(), ByteArray::BufferNodeSize * 3 - 1);
		EXPECT_EQ(array->writeableBytes(), ByteArray::BufferNodeSize + 1);
		static int64_t fillValue = 9223372000004775807;
		array->writeInt64(fillValue);
		--fillValue;
		EXPECT_EQ(array->readableBytes(), ByteArray::BufferNodeSize * 3 - 1 + sizeof(int64_t));
		EXPECT_EQ(array->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t) * 7);

		EXPECT_EQ(array->retrieveRaw(raw, ByteArray::BufferNodeSize * 3 - 1), ByteArray::BufferNodeSize * 3 - 1);
		EXPECT_EQ(array->readableBytes(), sizeof(int64_t));
		EXPECT_EQ(array->writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t) * 7);
	};
	fillByteArray(byteArray1);
	fillByteArray(byteArray2);
	{
		SequentialByteArrayReaderWithLock readVec({byteArray1, byteArray2});
		ASSERT_EQ(get_iovec_len(readVec), 4);
		EXPECT_NE(get_iovec(readVec), nullptr);
		int64_t data;
		std::memcpy(&data, get_iovec(readVec)[0].iov_base, sizeof(int8_t));
		std::memcpy(reinterpret_cast<char *>(&data) + 1, get_iovec(readVec)[1].iov_base, sizeof(int8_t) * 7);
		EXPECT_EQ(be64toh(data), 9223372000004775807);
		std::memcpy(&data, get_iovec(readVec)[2].iov_base, sizeof(int8_t));
		std::memcpy(reinterpret_cast<char *>(&data) + 1, get_iovec(readVec)[3].iov_base, sizeof(int8_t) * 7);
		EXPECT_EQ(be64toh(data), 9223372000004775806);
		readVec.adjustByteArray(sizeof(int64_t) * 2);
	}
	EXPECT_EQ(byteArray1->readableBytes(), 0);
	EXPECT_EQ(byteArray1->writeableBytes(), ByteArray::BufferNodeSize * 4 - sizeof(int8_t) * 7);
	EXPECT_EQ(byteArray2->readableBytes(), 0);
	EXPECT_EQ(byteArray2->writeableBytes(), ByteArray::BufferNodeSize * 4 - sizeof(int8_t) * 7);
}
