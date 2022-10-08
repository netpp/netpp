#include <gtest/gtest.h>
#include "buffer/ByteArray.h"

using namespace netpp;

class ByteArrayTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(ByteArrayTest, DefaultConstructedByteArray)
{
	ByteArray array;
	EXPECT_EQ(array.readableBytes(), 0);
}

TEST_F(ByteArrayTest, CopyByteArray)
{
	ByteArray source;
	source.writeInt8(1);
	source.writeInt16(2);
	ByteArray array(source);
	EXPECT_EQ(array.readableBytes(), sizeof(int8_t) + sizeof(int16_t));
	EXPECT_EQ(array.retrieveInt8(), 1);
	EXPECT_EQ(array.retrieveInt16(), 2);
}

TEST_F(ByteArrayTest, MoveByteArray)
{
	ByteArray source;
	source.writeInt8(1);
	source.writeInt16(2);
	ByteArray array(std::move(source));
	EXPECT_EQ(array.readableBytes(), sizeof(int8_t) + sizeof(int16_t));
	EXPECT_EQ(array.retrieveInt8(), 1);
	EXPECT_EQ(array.retrieveInt16(), 2);
}

TEST_F(ByteArrayTest, WriteInt8)
{
	ByteArray byteArray;
	EXPECT_EQ(byteArray.readableBytes(), 0);

	byteArray.writeInt8(1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray.retrieveInt8(), 1);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteInt16)
{
	ByteArray byteArray;
	byteArray.writeInt16(2);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int16_t));
	EXPECT_EQ(byteArray.retrieveInt16(), 2);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteInt32)
{
	ByteArray byteArray;
	byteArray.writeInt32(3);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int32_t));
	EXPECT_EQ(byteArray.retrieveInt32(), 3);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteInt64)
{
	ByteArray byteArray;
	byteArray.writeInt64(4);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray.retrieveInt64(), 4);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteUInt8)
{
	ByteArray byteArray;
	byteArray.writeUInt8(5);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint8_t));
	EXPECT_EQ(byteArray.retrieveUInt8(), 5);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteUInt16)
{
	ByteArray byteArray;
	byteArray.writeUInt16(6);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint16_t));
	EXPECT_EQ(byteArray.retrieveUInt16(), 6);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteUInt32)
{
	ByteArray byteArray;
	byteArray.writeUInt32(7);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint32_t));
	EXPECT_EQ(byteArray.retrieveUInt32(), 7);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteUInt64)
{
	ByteArray byteArray;
	byteArray.writeUInt64(8);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint64_t));
	EXPECT_EQ(byteArray.retrieveUInt64(), 8);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteFloat)
{
	ByteArray byteArray;
	byteArray.writeFloat(static_cast<float>(9.1));
	EXPECT_EQ(byteArray.readableBytes(), sizeof(float));
	EXPECT_FLOAT_EQ(byteArray.retrieveFloat(), static_cast<float>(9.1));
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteDouble)
{
	ByteArray byteArray;
	byteArray.writeDouble(10.1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(double));
	EXPECT_DOUBLE_EQ(byteArray.retrieveDouble(), static_cast<double>(10.1));
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteString)
{
	ByteArray byteArray;
	std::string testStr("11");
	std::size_t stringLength = testStr.length() * sizeof(char);
	byteArray.writeString(testStr);
	EXPECT_EQ(byteArray.readableBytes(), stringLength);
	EXPECT_EQ(byteArray.retrieveString(testStr.length()), testStr);
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, WriteRaw)
{
	ByteArray byteArray;
	std::size_t stringLength = sizeof(char) * 3;
	byteArray.writeRaw("12", 3);
	EXPECT_EQ(byteArray.readableBytes(), stringLength);
	char buffer[3];
	EXPECT_EQ(byteArray.retrieveRaw(buffer, 3), 3);
	EXPECT_STREQ(buffer, "12");
	EXPECT_EQ(byteArray.readableBytes(), 0);
}

TEST_F(ByteArrayTest, ReadEmpty)
{
	ByteArray byteArray;
	EXPECT_EQ(byteArray.readableBytes(), 0);

	int8_t i8t = byteArray.retrieveInt8();
	EXPECT_EQ(i8t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	int16_t i16t = byteArray.retrieveInt16();
	EXPECT_EQ(i16t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	int32_t i32t = byteArray.retrieveInt32();
	EXPECT_EQ(i32t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	int64_t i64t = byteArray.retrieveInt64();
	EXPECT_EQ(i64t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	
	uint8_t ui8t = byteArray.retrieveUInt8();
	EXPECT_EQ(ui8t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	uint16_t ui16t = byteArray.retrieveUInt16();
	EXPECT_EQ(ui16t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	uint32_t ui32t = byteArray.retrieveUInt32();
	EXPECT_EQ(ui32t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	uint64_t ui64t = byteArray.retrieveUInt64();
	EXPECT_EQ(ui64t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	float ft = byteArray.retrieveFloat();
	EXPECT_EQ(ft, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	double dt = byteArray.retrieveDouble();
	EXPECT_EQ(dt, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);

	std::string str1 = byteArray.retrieveString(0);
	EXPECT_EQ(str1, std::string(""));
	EXPECT_EQ(byteArray.readableBytes(), 0);

	std::string str2 = byteArray.retrieveString(100);
	EXPECT_EQ(str2, std::string(""));
	EXPECT_EQ(byteArray.readableBytes(), 0);
	
	char raw[100] = "";
	EXPECT_EQ(byteArray.retrieveRaw(raw, 100), 0);
	EXPECT_STREQ(raw, "");
	EXPECT_EQ(byteArray.readableBytes(), 0);
}
