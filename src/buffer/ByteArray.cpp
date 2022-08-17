#include <cstring>
#include "buffer/ByteArray.h"
#include "support/Log.h"

namespace netpp {
ByteArray::ByteArray()
	: m_availableSizeToRead{0}, m_availableSizeToWrite{CowBuffer::defaultNodeSize * BufferNodeSize},
	m_nodes{std::make_unique<CowBuffer>()}
{
	m_readNode = m_nodes->begin();
	m_writeNode = m_readNode;
}

ByteArray::ByteArray(const ByteArray &other)
{
	std::lock_guard lck(other.m_bufferMutex);
	m_availableSizeToRead = other.m_availableSizeToRead;
	m_availableSizeToWrite = other.m_availableSizeToWrite;
	m_nodes = std::make_unique<CowBuffer>(*other.m_nodes);
	m_readNode = CowBuffer::iterator(m_nodes.get(), other.m_readNode);
	m_writeNode = CowBuffer::iterator(m_nodes.get(), other.m_writeNode);
}

ByteArray::ByteArray(ByteArray &&other) noexcept
{
	std::lock_guard lck(other.m_bufferMutex);
	m_availableSizeToRead = other.m_availableSizeToRead;
	m_availableSizeToWrite = other.m_availableSizeToWrite;
	m_readNode = other.m_readNode;
	m_writeNode = other.m_writeNode;
	m_nodes = std::move(other.m_nodes);
}

ByteArray::ByteArray(ByteArray &other, LengthType size, bool move)
		: ByteArray(other)
{
	LengthType nodeMovement = (m_availableSizeToRead - size) / BufferNodeSize;

	if (m_availableSizeToRead >= size)
	{
		m_availableSizeToWrite += m_availableSizeToRead - size;
		m_availableSizeToRead = size;
		m_writeNode = m_writeNode - nodeMovement;
		m_writeNode->start = 0;
		m_writeNode->end = size % BufferNodeSize;
	}
	if (move)
	{
		other.m_availableSizeToRead -= size;
		other.m_readNode = m_readNode + nodeMovement;
		other.m_writeNode->start = size % BufferNodeSize;
		other.m_writeNode->end = size % BufferNodeSize;
	}
}

void ByteArray::writeInt8(int8_t value)
{
	writeRaw(reinterpret_cast<char *>(&value), sizeof(int8_t));
}

void ByteArray::writeInt16(int16_t value)
{
	uint16_t v = ::htobe16(static_cast<uint16_t>(value));
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int16_t));
}

void ByteArray::writeInt32(int32_t value)
{
	uint32_t v = ::htobe32(static_cast<uint32_t>(value));
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int32_t));
}

void ByteArray::writeInt64(int64_t value)
{
	uint64_t v = ::htobe64(static_cast<uint64_t>(value));
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int64_t));
}

void ByteArray::writeUInt8(uint8_t value)
{
	writeRaw(reinterpret_cast<char *>(&value), sizeof(uint8_t));
}

void ByteArray::writeUInt16(uint16_t value)
{
	uint16_t v = ::htobe16(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(uint16_t));
}

void ByteArray::writeUInt32(uint32_t value)
{
	uint32_t v = ::htobe32(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(uint32_t));
}

void ByteArray::writeUInt64(uint64_t value)
{
	uint64_t v = ::htobe64(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(uint64_t));
}

void ByteArray::writeFloat(float value)
{
	// must support IEEE
	writeRaw(reinterpret_cast<char *>(&value), sizeof(float));
}

void ByteArray::writeDouble(double value)
{
	// must support IEEE
	writeRaw(reinterpret_cast<char *>(&value), sizeof(double));
}

void ByteArray::writeString(const std::string &value)
{
	writeRaw(value.data(), sizeof(char) * (value.size()));
}

void ByteArray::writeRaw(const char *data, std::size_t length)
{
	LOG_TRACE("need to write {} bytes", length);

	std::lock_guard lck(m_bufferMutex);
	if (m_availableSizeToWrite <= length)
		unlockedAllocIfNotEnough(length);

	std::size_t expectWrite = length;
	auto it = m_writeNode;
	while (length > 0)
	{
		std::size_t bytesToWrite = BufferNodeSize - it->end;
		bytesToWrite = (bytesToWrite < length) ? bytesToWrite : length;
		std::memcpy(it->buffer + it->end, data, bytesToWrite);
		it->end += bytesToWrite;
		data += bytesToWrite;
		length -= bytesToWrite;
		// move to next node
		if (it->end == BufferNodeSize)
		{
			++it;
			if (it == m_nodes->end())
			{
				--it;
				break;
			}
		}
	}
	m_writeNode = it;
	m_availableSizeToRead += (expectWrite - length);
	m_availableSizeToWrite -= (expectWrite - length);
}

int8_t ByteArray::retrieveInt8()
{
	int8_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int8_t)) != sizeof(int8_t))
		return 0;
	return v;
}

int16_t ByteArray::retrieveInt16()
{
	uint16_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int16_t)) != sizeof(int16_t))
		return 0;
	return static_cast<int16_t>(::be16toh(v));
}

int32_t ByteArray::retrieveInt32()
{
	uint32_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int32_t)) != sizeof(int32_t))
		return 0;
	return static_cast<int32_t>(::be32toh(v));
}

int64_t ByteArray::retrieveInt64()
{
	uint64_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int64_t)) != sizeof(int64_t))
		return 0;
	return static_cast<int64_t>(::be64toh(v));
}

uint8_t ByteArray::retrieveUInt8()
{
	uint8_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint8_t)) != sizeof(uint8_t))
		return 0;
	return v;
}

uint16_t ByteArray::retrieveUInt16()
{
	uint16_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint16_t)) != sizeof(uint16_t))
		return 0;
	v = ::be16toh(v);
	return v;
}

uint32_t ByteArray::retrieveUInt32()
{
	uint32_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint32_t)) != sizeof(uint32_t))
		return 0;
	v = ::be32toh(v);
	return v;
}

uint64_t ByteArray::retrieveUInt64()
{
	uint64_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint64_t)) != sizeof(uint64_t))
		return 0;
	v = ::be64toh(v);
	return v;
}

float ByteArray::retrieveFloat()
{
	// must support IEEE
	float v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(float)) != sizeof(float))
		return 0.0;
	return v;
}

double ByteArray::retrieveDouble()
{
	// must support IEEE
	double v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(double)) != sizeof(double))
		return 0.0;
	return v;
}

std::string ByteArray::retrieveString(std::size_t length)
{
	std::string buffer;
	buffer.resize(length);
	if (retrieveRaw(buffer.data(), length) != 0)
		return buffer;
	else
		return "";
}

std::size_t ByteArray::retrieveRaw(char *buffer, std::size_t length)
{
	LOG_TRACE("need to retrieve {} bytes", length);
	char *bufferStartPtr = buffer;
	std::size_t expectToRead = length;
	
	std::lock_guard lck(m_bufferMutex);
	if (m_availableSizeToRead >= length)
	{
		auto it = m_readNode;
		while (length > 0)
		{
			std::size_t bytesToCopy = it->end - it->start;
			bytesToCopy = (bytesToCopy < length) ? bytesToCopy : length;
			std::memcpy(buffer, it->buffer + it->start, bytesToCopy);
			it->start += bytesToCopy;
			buffer += bytesToCopy;
			length -= bytesToCopy;
			if (it->start == BufferNodeSize)
				++it;
		}

		m_readNode = it;
		m_availableSizeToRead -= (expectToRead - length);
	}
	if (buffer > bufferStartPtr)
		return static_cast<std::size_t>(buffer - bufferStartPtr);
	else
		return 0;
}

void ByteArray::unlockedAllocIfNotEnough(std::size_t size)
{
	if (m_availableSizeToWrite <= size)
	{
		// move head first
		unlockedMoveBufferHead();

		if (m_availableSizeToWrite <= size)
		{
			CowBuffer::NodeContainerIndexer idleNodeCount = m_nodes->size() * 2;
			LengthType newWriteable = ((idleNodeCount - m_nodes->size()) * BufferNodeSize) + m_availableSizeToWrite;
			while (newWriteable <= size)
			{
				idleNodeCount *= 2;
				newWriteable = ((idleNodeCount - m_nodes->size()) * BufferNodeSize) + m_availableSizeToWrite;
			}
			m_nodes->allocMore(idleNodeCount);
			m_availableSizeToWrite = newWriteable;
		}
	}
}

void ByteArray::unlockedMoveBufferHead()
{
	if (m_readNode == m_nodes->begin())
		return;

	for (auto it = m_nodes->begin(); it != m_readNode; ++it)
	{
		it->start = 0;
		it->end = 0;
		--m_writeNode;
		m_availableSizeToWrite += BufferNodeSize;
	}
	m_nodes->moveToTail(m_readNode);
	m_readNode = m_nodes->begin();
}
}
