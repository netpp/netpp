//
// Created by 17271 on 2022/10/2.
//

#include "buffer/ByteArray.h"
#include <utility>
#include <cstring>
#include "buffer/Buffer.h"

namespace netpp {
LittleEndianBufferWrapper::LittleEndianBufferWrapper() = default;

void LittleEndianBufferWrapper::writeInt8(std::int8_t value)
{
	writeRaw(reinterpret_cast<char *>(&value), sizeof(int8_t));
}

void LittleEndianBufferWrapper::writeInt16(std::int16_t value)
{
	uint16_t v = ::htobe16(static_cast<uint16_t>(value));
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int16_t));
}

void LittleEndianBufferWrapper::writeInt32(std::int32_t value)
{
	uint32_t v = ::htobe32(static_cast<uint32_t>(value));
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int32_t));
}

void LittleEndianBufferWrapper::writeInt64(std::int64_t value)
{
	uint64_t v = ::htobe64(static_cast<uint64_t>(value));
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int64_t));
}

void LittleEndianBufferWrapper::writeUInt8(std::uint8_t value)
{
	writeRaw(reinterpret_cast<char *>(&value), sizeof(uint8_t));
}

void LittleEndianBufferWrapper::writeUInt16(std::uint16_t value)
{
	uint16_t v = ::htobe16(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(uint16_t));
}

void LittleEndianBufferWrapper::writeUInt32(std::uint32_t value)
{
	uint32_t v = ::htobe32(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(uint32_t));
}

void LittleEndianBufferWrapper::writeUInt64(std::uint64_t value)
{
	uint64_t v = ::htobe64(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(uint64_t));
}

void LittleEndianBufferWrapper::writeFloat(float value)
{
	// must support IEEE
	writeRaw(reinterpret_cast<char *>(&value), sizeof(float));
}

void LittleEndianBufferWrapper::writeDouble(double value)
{
	// must support IEEE
	writeRaw(reinterpret_cast<char *>(&value), sizeof(double));
}

void LittleEndianBufferWrapper::writeString(const std::string &value)
{
	writeRaw(value.data(), sizeof(char) * (value.size()));
}

std::int8_t LittleEndianBufferWrapper::retrieveInt8()
{
	int8_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int8_t)) != sizeof(int8_t))
		return 0;
	return v;
}

std::int16_t LittleEndianBufferWrapper::retrieveInt16()
{
	uint16_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int16_t)) != sizeof(int16_t))
		return 0;
	return static_cast<int16_t>(::be16toh(v));
}

std::int32_t LittleEndianBufferWrapper::retrieveInt32()
{
	uint32_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int32_t)) != sizeof(int32_t))
		return 0;
	return static_cast<int32_t>(::be32toh(v));
}

std::int64_t LittleEndianBufferWrapper::retrieveInt64()
{
	uint64_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int64_t)) != sizeof(int64_t))
		return 0;
	return static_cast<int64_t>(::be64toh(v));
}

std::uint8_t LittleEndianBufferWrapper::retrieveUInt8()
{
	uint8_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint8_t)) != sizeof(uint8_t))
		return 0;
	return v;
}

std::uint16_t LittleEndianBufferWrapper::retrieveUInt16()
{
	uint16_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint16_t)) != sizeof(uint16_t))
		return 0;
	v = ::be16toh(v);
	return v;
}

std::uint32_t LittleEndianBufferWrapper::retrieveUInt32()
{
	uint32_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint32_t)) != sizeof(uint32_t))
		return 0;
	v = ::be32toh(v);
	return v;
}

std::uint64_t LittleEndianBufferWrapper::retrieveUInt64()
{
	uint64_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint64_t)) != sizeof(uint64_t))
		return 0;
	v = ::be64toh(v);
	return v;
}

float LittleEndianBufferWrapper::retrieveFloat()
{
	// must support IEEE
	float v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(float)) != sizeof(float))
		return 0.0;
	return v;
}

double LittleEndianBufferWrapper::retrieveDouble()
{
	// must support IEEE
	double v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(double)) != sizeof(double))
		return 0.0;
	return v;
}

std::string LittleEndianBufferWrapper::retrieveString(BufferLength length)
{
	std::string buffer;
	buffer.resize(length);
	if (retrieveRaw(buffer.data(), length) != 0)
		return buffer;
	else
		return "";
}

ByteArray::ByteArray()
	: m_buffer{std::make_shared<Buffer>()}
{}

ByteArray::ByteArray(std::shared_ptr<Buffer> buffer)
	: m_buffer{std::move(buffer)}
{}

ByteArray::~ByteArray() = default;

void ByteArray::writeRaw(const char *data, BufferLength length)
{
	m_buffer->write(data, length);
}

BufferLength ByteArray::retrieveRaw(char *buffer, BufferLength length)
{
	return m_buffer->retrieve(buffer, length);
}

BufferLength ByteArray::readableBytes() const
{
	return m_buffer->readableBytes();
}

ByteArrayPeeker::ByteArrayPeeker()
	: m_peekPos{0}, m_buffer{std::make_shared<Buffer>()}
{}

ByteArrayPeeker::ByteArrayPeeker(std::shared_ptr<Buffer> buffer)
		: m_peekPos{0}, m_buffer{std::move(buffer)}
{}

ByteArrayPeeker::~ByteArrayPeeker() = default;

void ByteArrayPeeker::writeRaw(const char *data, BufferLength length)
{
	m_buffer->write(data, length);
}

BufferLength ByteArrayPeeker::retrieveRaw(char *buffer, BufferLength length)
{
	auto rawBuffer = m_buffer->getRawPointer();
	BufferLength lengthToCopy = m_buffer->readableBytes();
	if (lengthToCopy > length)
		lengthToCopy = length;
	std::memcpy(buffer, rawBuffer + m_peekPos, lengthToCopy);
	return lengthToCopy;
}

BufferLength ByteArrayPeeker::readableBytes() const
{
	return m_buffer->readableBytes();
}
} // netpp