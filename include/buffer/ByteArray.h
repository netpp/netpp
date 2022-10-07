//
// Created by gaojian on 2022/10/2.
//

#ifndef NETPP_BYTEARRAY_H
#define NETPP_BYTEARRAY_H

#include <cstdint>
#include "support/Types.h"

namespace netpp {
class Buffer;
class LittleEndianBufferWrapper {
public:
	/**
	 * @brief Default ByteArray
	 */
	LittleEndianBufferWrapper();

	virtual ~LittleEndianBufferWrapper() = default;

	/**
	 * @brief Write an int8 value to ByteArray
	 * @param value The value to write
	 */
	void writeInt8(std::int8_t value);
	/**
	 * @brief Write an int16 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeInt16(std::int16_t value);
	/**
	 * @brief Write an int32 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeInt32(std::int32_t value);
	/**
	 * @brief Write an int64 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeInt64(std::int64_t value);
	/**
	 * @brief Write a uint8 value to ByteArray
	 * @param value The value to write
	 */
	void writeUInt8(std::uint8_t value);
	/**
	 * @brief Write a uint16 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeUInt16(std::uint16_t value);
	/**
	 * @brief Write a uint32 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeUInt32(std::uint32_t value);
	/**
	 * @brief Write a uint64 value to ByteArray, the value will be stored in network-ending
	 * @param value The value to write
	 */
	void writeUInt64(std::uint64_t value);
	/**
	 * @brief Write a float value to ByteArray
	 * @note byte order not converted
	 * @param value The value to write
	 */
	void writeFloat(float value);
	/**
	 * @brief Write a float value to ByteArray
	 * @note byte order not converted
	 * @param value The value to write
	 */
	void writeDouble(double value);
	/**
	 * @brief Write a string to ByteArray
	 * @param value The value to write
	 */
	void writeString(const std::string &value);
	/**
	 * @brief Write raw data to ByteArray
	 * @param data The raw data
	 * @param length The size of data
	 */
	virtual void writeRaw(const char *data, BufferLength length) = 0;

	/**
	 * @brief Read 1 byte, convert it to int8
	 * @return 0 if readable size less than 1 byte
	 * @return otherwise return the value force convert to int8
	 */
	std::int8_t retrieveInt8();
	/**
	 * @brief Read 2 bytes, convert it to int16
	 * @return 0 if readable size less than 2 bytes
	 * @return otherwise read 2 bytes, convert network-ending to local-ending, cast int16
	 */
	std::int16_t retrieveInt16();
	/**
	 * @brief Read 4 bytes, convert it to int32
	 * @return 0 if readable size less than 4 bytes
	 * @return otherwise read 4 bytes, convert network-ending to local-ending, cast int32
	 */
	std::int32_t retrieveInt32();
	/**
	 * @brief Read 8 bytes, convert it to int64
	 * @return 0 if readable size less than 8 byte
	 * @return otherwise read 8 bytes, convert network-ending to local-ending, cast int64
	 */
	std::int64_t retrieveInt64();
	/**
	 * @brief Read 1 byte, convert it to uint8
	 * @return 0 if readable size less than 1 byte
	 * @return otherwise return the value force convert to uint8
	 */
	std::uint8_t retrieveUInt8();
	/**
	 * @brief Read 2 bytes, convert it to uint16
	 * @return 0 if readable size less than 2 bytes
	 * @return otherwise read 2 bytes, convert network-ending to local-ending, cast uint16
	 */
	std::uint16_t retrieveUInt16();
	/**
	 * @brief Read 4 bytes, convert it to uint32
	 * @return 0 if readable size less than 4 bytes
	 * @return otherwise read 4 bytes, convert network-ending to local-ending, cast uint32
	 */
	std::uint32_t retrieveUInt32();
	/**
	 * @brief Read 8 bytes, convert it to uint64
	 * @return 0 if readable size less than 8 bytes
	 * @return otherwise read 8 bytes, convert network-ending to local-ending, cast uint64
	 */
	std::uint64_t retrieveUInt64();
	/**
	 * @brief Read 4 bytes, convert it to float
	 * @return 0 if readable size less than 4 bytes
	 * @return otherwise read 4 bytes, not ending convert performed, cast float
	 */
	float retrieveFloat();
	/**
	 * @brief Read 8 bytes, convert it to double
	 * @return 0 if readable size less than 8 bytes
	 * @return otherwise read 8 bytes, not ending convert performed, cast double
	 */
	double retrieveDouble();
	/**
	 * @brief Read a string
	 * @param length The bytes to read
	 * @return A string
	 */
	std::string retrieveString(BufferLength length);
	/**
	 * @brief Retrieve raw data from ByteArray to buffer
	 * @param buffer		Store data here
	 * @param length		How many bytes to retrieve
	 * @return BufferLength	The actual size retrieved from ByteArray
	 */
	virtual BufferLength retrieveRaw(char *buffer, BufferLength length) = 0;

	/**
	 * @brief The readable bytes in buffer. From ReadNode's start to WriteNode's end.
	 */
	[[nodiscard]] virtual BufferLength readableBytes() const = 0;
};

class ByteArray : public LittleEndianBufferWrapper {
	friend std::shared_ptr<Buffer> extractBuffer(const ByteArray *byteArray);
public:
	ByteArray();
	explicit ByteArray(std::shared_ptr<Buffer> buffer);
	~ByteArray() override;

	void writeRaw(const char *data, netpp::BufferLength length) override;
	BufferLength retrieveRaw(char *buffer, netpp::BufferLength length) override;
	[[nodiscard]] BufferLength readableBytes() const override;

private:
	std::shared_ptr<Buffer> m_buffer;
};

class ByteArrayPeeker : public LittleEndianBufferWrapper {
public:
	ByteArrayPeeker();
	explicit ByteArrayPeeker(std::shared_ptr<Buffer> buffer);
	~ByteArrayPeeker() override;

	void writeRaw(const char *data, netpp::BufferLength length) override;
	BufferLength retrieveRaw(char *buffer, netpp::BufferLength length) override;
	[[nodiscard]] BufferLength readableBytes() const override;

private:
	BufferLength m_peekPos;
	std::shared_ptr<Buffer> m_buffer;
};
} // netpp

#endif //NETPP_BYTEARRAY_H
