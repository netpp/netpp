#include "ByteArray.h"
#include <cstring>
#include "support/Log.h"

namespace netpp {
ByteArray::ByteArray()
	: m_availableSizeToRead{0}, m_availableSizeToWrite{BufferNode::BufferSize},
	m_nodeCount{1}, 
	m_bufferHead{std::make_shared<BufferNode>()}
{
	_bufferTail = m_bufferHead;
	_currentReadBufferNode = m_bufferHead;
	_currentWriteBufferNode = m_bufferHead;
}

void ByteArray::writeInt8(int8_t value)
{
	writeRaw(reinterpret_cast<char *>(&value), sizeof(int8_t));
}

void ByteArray::writeInt16(int16_t value)
{
	int16_t v = htobe16(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int16_t));
}

void ByteArray::writeInt32(int32_t value)
{
	int32_t v = htobe32(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int32_t));
}

void ByteArray::writeInt64(int64_t value)
{
	int64_t v = htobe64(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int64_t));
}

void ByteArray::writeUInt8(uint8_t value)
{
	writeRaw(reinterpret_cast<char *>(&value), sizeof(uint8_t));
}

void ByteArray::writeUInt16(uint16_t value)
{
	uint16_t v = htobe16(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(uint16_t));
}

void ByteArray::writeUInt32(uint32_t value)
{
	uint32_t v = htobe32(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(uint32_t));
}

void ByteArray::writeUInt64(uint64_t value)
{
	uint64_t v = htobe64(value);
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

void ByteArray::writeString(std::string value)
{
	writeRaw(value.data(), sizeof(char) * (value.size()));
}

void ByteArray::writeRaw(const char *data, std::size_t length)
{
	LOG_TRACE("need to write {} bytes", length);

	std::lock_guard lck(m_bufferMutex);
	std::shared_ptr<BufferNode> node = _currentWriteBufferNode.lock();
	std::size_t availableSize = BufferNode::BufferSize - node->end;
	if (availableSize <= length)
		unlockedAllocIfNotEnough(length);
	
	// when (availableSize == length), current node is full,
	// move _currentWriteBufferNode to next node
	while (availableSize <= length)
	{
		std::memcpy(node->buffer + node->end, data, availableSize);
		node->end = BufferNode::BufferSize;
		data += availableSize;
		length -= availableSize;
		m_availableSizeToRead += availableSize;
		m_availableSizeToWrite -= availableSize;
		node = node->next;
		_currentWriteBufferNode = node;
		availableSize = BufferNode::BufferSize;
	}
	if (length > 0)
	{
		std::memcpy(node->buffer + node->end, data, length);
		node->end += length;
		m_availableSizeToRead += length;
		m_availableSizeToWrite -= length;
	}
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
	int16_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int16_t)) != sizeof(int16_t))
		return 0;
	v = be16toh(v);
	return v;
}

int32_t ByteArray::retrieveInt32()
{
	int32_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int32_t)) != sizeof(int32_t))
		return 0;
	v = be32toh(v);
	return v;
}

int64_t ByteArray::retrieveInt64()
{
	int64_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int64_t)) != sizeof(int64_t))
		return 0;
	v = be64toh(v);
	return v;
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
	v = be16toh(v);
	return v;
}

uint32_t ByteArray::retrieveUInt32()
{
	uint32_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint32_t)) != sizeof(uint32_t))
		return 0;
	v = be32toh(v);
	return v;
}

uint64_t ByteArray::retrieveUInt64()
{
	uint64_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(uint64_t)) != sizeof(uint64_t))
		return 0;
	v = be64toh(v);
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
	retrieveRaw(buffer.data(), length);
	return buffer;
}

std::size_t ByteArray::retrieveRaw(char *buffer, std::size_t length)
{
	LOG_TRACE("need to retrieve {} bytes", length);
	char *bufferStartPtr = buffer;
	
	std::lock_guard lck(m_bufferMutex);
	std::shared_ptr<BufferNode> node = _currentReadBufferNode.lock();
	std::size_t usedBytesInNode = node->end - node->start;		// how many bytes used in this node
	bool notEnoughtBytesInLastNode = false;
	while (usedBytesInNode <= length)
	{
		std::memcpy(buffer, node->buffer + node->start, usedBytesInNode);
		node->start = node->end;
		buffer += usedBytesInNode;
		length -= usedBytesInNode;
		m_availableSizeToRead -= usedBytesInNode;
		// read from buffuer do not change m_availableSizeToWrite
		if (node->next)		// has next node
		{
			_currentReadBufferNode = node;
			node = node->next;
			usedBytesInNode = node->end - node->start;
		}
		else
		{
			// do not have next node, and current node readable bytes is less than length,
			// mark it as end
			notEnoughtBytesInLastNode = true;
			break;
		}
	}
	if (!notEnoughtBytesInLastNode && length > 0)	// the last node
	{
		std::memcpy(buffer, node->buffer + node->start, length);
		node->start += length;
		m_availableSizeToRead -= length;
		buffer += length;
	}
	return buffer - bufferStartPtr;
}

void ByteArray::unlockedAllocIfNotEnough(std::size_t size)
{
	if (m_availableSizeToWrite <= size)
	{
		moveBufferHead();
		// if still not enough
		std::shared_ptr<BufferNode> tail = _bufferTail.lock();
		while (m_availableSizeToWrite <= size)
		{
			for (unsigned i = m_nodeCount; i < m_nodeCount * 2; ++i, tail = tail->next)
				tail->next = std::make_shared<BufferNode>();
			m_availableSizeToWrite += m_nodeCount * BufferNode::BufferSize;
			m_nodeCount *= 2;
		}
		_bufferTail = tail;
	}
}

void ByteArray::moveBufferHead()
{
	std::shared_ptr<BufferNode> tail = _bufferTail.lock();
	std::shared_ptr<BufferNode> readNode = _currentReadBufferNode.lock();
	// move unused buffer node to end
	if (m_bufferHead != readNode)
	{
		// the first node
		m_availableSizeToWrite += BufferNode::BufferSize;
		m_bufferHead->start = 0;
		m_bufferHead->end = 0;
		// second to _currentReadBufferNode
		std::shared_ptr<BufferNode> node = m_bufferHead;
		while (node->next != readNode)
		{
			m_availableSizeToWrite += BufferNode::BufferSize;
			node->start = 0;
			node->end = 0;
			node = node->next;
		}
		tail->next = m_bufferHead;
		m_bufferHead = readNode;
		node->next = nullptr;
		_bufferTail = node;
	}
}

// no need to init buffer
ByteArray::BufferNode::BufferNode()
	: start{0}, end{0}, next{nullptr}
{}
}
