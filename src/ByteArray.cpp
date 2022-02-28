#include <cstring>
#include "ByteArray.h"
#include "internal/support/Log.h"

namespace netpp {
ByteArray::ByteArray()
	: m_availableSizeToRead{0}, m_availableSizeToWrite{CowBuffer::defaultNodeSize * BufferNodeSize},
	m_readNode{0}, m_writeNode{0}, m_nodes{std::make_unique<CowBuffer>()}
{}

ByteArray::ByteArray(const ByteArray &other)
{
	std::lock_guard lck(other.m_bufferMutex);
	m_availableSizeToRead = other.m_availableSizeToRead;
	m_availableSizeToWrite = other.m_availableSizeToWrite;
	m_readNode = other.m_readNode;
	m_writeNode = other.m_writeNode;
	m_nodes = std::make_unique<CowBuffer>(*other.m_nodes);
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
	auto end = m_nodes->cowEnd(m_nodes->size());
	for (auto it = m_nodes->cowBegin(m_writeNode); length > 0 && (it <=> end) < 0; ++it)
	{
		std::size_t bytesToWrite = BufferNodeSize - it->end;
		bytesToWrite = (bytesToWrite < length) ? bytesToWrite : length;
		std::memcpy(it->buffer + it->end, data, bytesToWrite);
		it->end += bytesToWrite;
		data += bytesToWrite;
		length -= bytesToWrite;
		// have next iteration
		if ((m_writeNode + 1 < m_nodes->size()) && (it->end == BufferNodeSize))
			++m_writeNode;
	}
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
		ByteArray::CowBuffer::NodeContainerIndexer endOfRead = endOfReadNode();
		for (auto it = m_nodes->cowBegin(m_readNode); length > 0 && (it <=> m_nodes->cowEnd(endOfRead)) < 0; ++it)
		{
			std::size_t bytesToCopy = it->end - it->start;
			bytesToCopy = (bytesToCopy < length) ? bytesToCopy : length;
			std::memcpy(buffer, it->buffer + it->start, bytesToCopy);
			it->start += bytesToCopy;
			buffer += bytesToCopy;
			length -= bytesToCopy;
			// if this node is empty, and not the last node
			if (it->start == BufferNodeSize && m_readNode + 1 < m_nodes->size())
			{
				// read and write point at same node, move write node also
				if (m_readNode == m_writeNode)
					++m_writeNode;
				++m_readNode;
			}
		}
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
	if (m_readNode == 0)
		return;

	m_nodes->moveToTail(m_readNode, [](internal::buffer::BufferNode &node) { node.start = 0; node.end = 0; });
	m_availableSizeToWrite += m_readNode * BufferNodeSize;
	m_writeNode -= m_readNode;
	m_readNode = 0;
}

ByteArray::CowBuffer::NodeContainerIndexer ByteArray::endOfReadNode() const
{
	ByteArray::CowBuffer::NodeContainerIndexer endOfRead;
	if (m_writeNode == m_readNode)
	{
		endOfRead = m_readNode + 1;
	}
	else
	{
		auto writeNode = m_nodes->constBegin(m_writeNode);
		if (writeNode->end == 0)
			endOfRead = m_writeNode;
		else
			endOfRead = m_writeNode + 1;
	}
	return endOfRead;
}
}
