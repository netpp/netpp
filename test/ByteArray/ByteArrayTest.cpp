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

using namespace netpp;

class ByteArrayTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(ByteArrayTest, WriteInt8)
{
	ByteArray byteArray;
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	byteArray.writeInt8(1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));
	EXPECT_EQ(byteArray.retrieveInt8(), 1);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int8_t));
}

TEST_F(ByteArrayTest, WriteInt16)
{
	ByteArray byteArray;
	byteArray.writeInt16(2);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int16_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int16_t));
	EXPECT_EQ(byteArray.retrieveInt16(), 2);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int16_t));
}

TEST_F(ByteArrayTest, WriteInt32)
{
	ByteArray byteArray;
	byteArray.writeInt32(3);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int32_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int32_t));
	EXPECT_EQ(byteArray.retrieveInt32(), 3);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int32_t));
}

TEST_F(ByteArrayTest, WriteInt64)
{
	ByteArray byteArray;
	byteArray.writeInt64(4);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int64_t));
	EXPECT_EQ(byteArray.retrieveInt64(), 4);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int64_t));
}

TEST_F(ByteArrayTest, WriteUInt8)
{
	ByteArray byteArray;
	byteArray.writeUInt8(5);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint8_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(uint8_t));
	EXPECT_EQ(byteArray.retrieveUInt8(), 5);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(uint8_t));
}

TEST_F(ByteArrayTest, WriteUInt16)
{
	ByteArray byteArray;
	byteArray.writeUInt16(6);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint16_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(uint16_t));
	EXPECT_EQ(byteArray.retrieveUInt16(), 6);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(uint16_t));
}

TEST_F(ByteArrayTest, WriteUInt32)
{
	ByteArray byteArray;
	byteArray.writeUInt32(7);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint32_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(uint32_t));
	EXPECT_EQ(byteArray.retrieveUInt32(), 7);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(uint32_t));
}

TEST_F(ByteArrayTest, WriteUInt64)
{
	ByteArray byteArray;
	byteArray.writeUInt64(8);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(uint64_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(uint64_t));
	EXPECT_EQ(byteArray.retrieveUInt64(), 8);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(uint64_t));
}

TEST_F(ByteArrayTest, WriteFloat)
{
	ByteArray byteArray;
	byteArray.writeFloat(9.1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(float));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(float));
	EXPECT_FLOAT_EQ(byteArray.retrieveFloat(), 9.1);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(float));
}

TEST_F(ByteArrayTest, WriteDouble)
{
	ByteArray byteArray;
	byteArray.writeDouble(10.1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(double));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(double));
	EXPECT_FLOAT_EQ(byteArray.retrieveDouble(), 10.1);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(double));
}

TEST_F(ByteArrayTest, WriteString)
{
	ByteArray byteArray;
	std::string testStr("11");
	std::size_t stringLength = testStr.length() * sizeof(char);
	byteArray.writeString(testStr);
	EXPECT_EQ(byteArray.readableBytes(), stringLength);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - stringLength);
	EXPECT_EQ(byteArray.retrieveString(testStr.length()), testStr);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - stringLength);
}

TEST_F(ByteArrayTest, WriteRaw)
{
	ByteArray byteArray;
	std::size_t stringLength = sizeof(char) * 3;
	byteArray.writeRaw("12", 3);
	EXPECT_EQ(byteArray.readableBytes(), stringLength);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - stringLength);
	char buffer[3];
	EXPECT_EQ(byteArray.retrieveRaw(buffer, 3), 3);
	EXPECT_STREQ(buffer, "12");
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - stringLength);
}

TEST_F(ByteArrayTest, ReadEmpty)
{
	ByteArray byteArray;
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	int8_t i8t = byteArray.retrieveInt8();
	EXPECT_EQ(i8t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	int16_t i16t = byteArray.retrieveInt16();
	EXPECT_EQ(i16t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	int32_t i32t = byteArray.retrieveInt32();
	EXPECT_EQ(i32t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	int64_t i64t = byteArray.retrieveInt64();
	EXPECT_EQ(i64t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);
	
	uint8_t ui8t = byteArray.retrieveUInt8();
	EXPECT_EQ(ui8t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	uint16_t ui16t = byteArray.retrieveUInt16();
	EXPECT_EQ(ui16t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	uint32_t ui32t = byteArray.retrieveUInt32();
	EXPECT_EQ(ui32t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	uint64_t ui64t = byteArray.retrieveUInt64();
	EXPECT_EQ(ui64t, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	float ft = byteArray.retrieveFloat();
	EXPECT_EQ(ft, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	double dt = byteArray.retrieveDouble();
	EXPECT_EQ(dt, 0);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	std::string str = byteArray.retrieveString(ByteArray::BufferNodeSize);
	EXPECT_EQ(str, std::string(""));
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);
	
	char raw[ByteArray::BufferNodeSize] = "";
	EXPECT_EQ(byteArray.retrieveRaw(raw, ByteArray::BufferNodeSize), 0);
	EXPECT_STREQ(raw, "");
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);
}

TEST_F(ByteArrayTest, MoveBufferNode)
{
	ByteArray byteArray;
	char raw[ByteArray::BufferNodeSize] = {'\0'};
	byteArray.writeRaw(raw, ByteArray::BufferNodeSize);	// alloc new node
	EXPECT_EQ(byteArray.readableBytes(), ByteArray::BufferNodeSize);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	EXPECT_EQ(byteArray.retrieveRaw(raw, ByteArray::BufferNodeSize), ByteArray::BufferNodeSize);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	byteArray.writeRaw(raw, ByteArray::BufferNodeSize);	// head node will move to tail
	EXPECT_EQ(byteArray.readableBytes(), ByteArray::BufferNodeSize);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	EXPECT_EQ(byteArray.retrieveRaw(raw, ByteArray::BufferNodeSize), ByteArray::BufferNodeSize);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);

	byteArray.writeInt8(1);
	EXPECT_EQ(byteArray.readableBytes(), 1);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - 1);

	EXPECT_EQ(byteArray.retrieveInt8(), 1);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - 1);
}

TEST_F(ByteArrayTest, CrossNodeReadWriteBigEntry)
{
	ByteArray byteArray;
	char raw[ByteArray::BufferNodeSize * 10] = {'\0'};
	byteArray.writeRaw(raw, ByteArray::BufferNodeSize * 10);	// 16 nodes, last 6 is usable
	EXPECT_EQ(byteArray.readableBytes(), ByteArray::BufferNodeSize * 10);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize * 6);
	EXPECT_EQ(byteArray.retrieveRaw(raw, ByteArray::BufferNodeSize * 10), ByteArray::BufferNodeSize * 10);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize * 6);

	byteArray.writeRaw(raw, ByteArray::BufferNodeSize * 6);	// move head, 10 node is usable
	EXPECT_EQ(byteArray.readableBytes(), ByteArray::BufferNodeSize * 6);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize * 10);
	EXPECT_EQ(byteArray.retrieveRaw(raw, ByteArray::BufferNodeSize * 6), ByteArray::BufferNodeSize * 6);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize * 10);
}

TEST_F(ByteArrayTest, CrossNodeReadWriteInt8)
{
	ByteArray byteArray;
	char raw[ByteArray::BufferNodeSize] = {'\0'};
	byteArray.writeRaw(raw, ByteArray::BufferNodeSize - 1);	// write 1023 bytes
	EXPECT_EQ(byteArray.readableBytes(), ByteArray::BufferNodeSize - 1);
	EXPECT_EQ(byteArray.writeableBytes(), 1);

	byteArray.writeInt8(1);		// write 1 byte, new node alloc
	EXPECT_EQ(byteArray.readableBytes(), ByteArray::BufferNodeSize);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize);
	byteArray.writeInt8(2);		// write 1 byte
	EXPECT_EQ(byteArray.readableBytes(), ByteArray::BufferNodeSize + 1);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - 1);

	EXPECT_EQ(byteArray.retrieveRaw(raw, ByteArray::BufferNodeSize - 1), ByteArray::BufferNodeSize - 1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t) * 2);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - 1);

	EXPECT_EQ(byteArray.retrieveInt8(), 1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int8_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - 1);

	EXPECT_EQ(byteArray.retrieveInt8(), 2);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - 1);
}

TEST_F(ByteArrayTest, CrossNodeReadWriteInt64)
{
	ByteArray byteArray;
	char raw[ByteArray::BufferNodeSize] = {'\0'};
	byteArray.writeRaw(raw, ByteArray::BufferNodeSize - 1);	// write 1023 bytes
	EXPECT_EQ(byteArray.readableBytes(), ByteArray::BufferNodeSize - 1);
	EXPECT_EQ(byteArray.writeableBytes(), 1);

	byteArray.writeInt64(9223372000004775807);		// new node alloc
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int64_t) + 1);
	EXPECT_EQ(byteArray.readableBytes(), ByteArray::BufferNodeSize + sizeof(int64_t) - 1);

	EXPECT_EQ(byteArray.retrieveRaw(raw, ByteArray::BufferNodeSize - 1), ByteArray::BufferNodeSize - 1);
	EXPECT_EQ(byteArray.readableBytes(), sizeof(int64_t));
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int64_t) + 1);

	EXPECT_EQ(byteArray.retrieveInt64(), 9223372000004775807);
	EXPECT_EQ(byteArray.readableBytes(), 0);
	EXPECT_EQ(byteArray.writeableBytes(), ByteArray::BufferNodeSize - sizeof(int64_t) + 1);
}

#pragma GCC diagnostic pop
