#include "ByteArray.h"
#include <cstring>
#include "internal/support/Log.h"
#include <cassert>

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
void ByteArray::writeInt16(int16_t value)
{
	int16_t v = ::htobe16(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int16_t));
}

void ByteArray::writeInt32(int32_t value)
{
	int32_t v = ::htobe32(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int32_t));
}

void ByteArray::writeInt64(int64_t value)
{
	int64_t v = ::htobe64(value);
	writeRaw(reinterpret_cast<char *>(&v), sizeof(int64_t));
}
#pragma GCC diagnostic pop

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
	std::shared_ptr<BufferNode> node = _currentWriteBufferNode.lock();
	std::size_t availableSize = BufferNode::BufferSize - node->end;
	if (availableSize <= length)
		unlockedAllocIfNotEnough(length);
	
	std::size_t bytesToWrite = (availableSize < length) ? availableSize : length;
	while (length > 0)
	{
		std::memcpy(node->buffer + node->end, data, bytesToWrite);
		node->end += bytesToWrite;
		data += bytesToWrite;
		length -= bytesToWrite;
		m_availableSizeToRead += bytesToWrite;
		m_availableSizeToWrite -= bytesToWrite;
		node = node->next;
		// has more to write
		// this node is full
		if (node && (length != 0 || node->end == BufferNode::BufferSize))
		{
			_currentWriteBufferNode = node;
			availableSize = BufferNode::BufferSize;
			bytesToWrite = (availableSize < length) ? availableSize : length;
		}
	}
}

int8_t ByteArray::retrieveInt8()
{
	int8_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int8_t)) != sizeof(int8_t))
		return 0;
	return v;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
int16_t ByteArray::retrieveInt16()
{
	int16_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int16_t)) != sizeof(int16_t))
		return 0;
	v = ::be16toh(v);
	return v;
}

int32_t ByteArray::retrieveInt32()
{
	int32_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int32_t)) != sizeof(int32_t))
		return 0;
	v = ::be32toh(v);
	return v;
}

int64_t ByteArray::retrieveInt64()
{
	int64_t v;
	if (retrieveRaw(reinterpret_cast<char *>(&v), sizeof(int64_t)) != sizeof(int64_t))
		return 0;
	v = ::be64toh(v);
	return v;
}
#pragma GCC diagnostic pop

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
	std::size_t bytesToCopy = (usedBytesInNode < length) ? usedBytesInNode : length;
	while (node && length > 0)
	{
		std::memcpy(buffer, node->buffer + node->start, bytesToCopy);
		node->start += bytesToCopy;
		buffer += bytesToCopy;
		length -= bytesToCopy;
		m_availableSizeToRead -= bytesToCopy;
		// is current node empty(start pointer point reach max size)
		bool readOut = (node->start == BufferNode::BufferSize);
		// if read node moved, move write node also
		bool mayBeMoveWriteNode = (node == _currentWriteBufferNode.lock());
		node = node->next;
		// move ReadNode when
		// 1.has next node
		// 2.this node is empty
		if (node && readOut)
		{
			// move cross write node
			if (mayBeMoveWriteNode)
				_currentWriteBufferNode = node;
			_currentReadBufferNode = node;
			usedBytesInNode = node->end - node->start;
			bytesToCopy = (usedBytesInNode < length) ? usedBytesInNode : length;
		}
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
		unlockedMoveBufferHead();
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

void ByteArray::unlockedMoveBufferHead()
{
	std::shared_ptr<BufferNode> readNode = _currentReadBufferNode.lock();
	if (readNode == m_bufferHead)
		return;
	// move unused buffer node to end
	std::shared_ptr<BufferNode> node = m_bufferHead;
	std::shared_ptr<BufferNode> lastNode = m_bufferHead;
	while (node && node != readNode)
	{
		m_availableSizeToWrite += BufferNode::BufferSize;
		node->start = 0;
		node->end = 0;
		lastNode = node;
		node = node->next;
	}
	std::shared_ptr<BufferNode> tail = _bufferTail.lock();
	// readNode might be head(only one node in list), node not point to next
	// if node moved, node will never equal to readNode
	if (lastNode != readNode)
	{
		tail->next = m_bufferHead;
		m_bufferHead = readNode;
		lastNode->next = nullptr;
		_bufferTail = lastNode;
	}
}

ByteArray::BufferNode::BufferNode()
	: start{0}, end{0}, buffer{}, next{nullptr}
{}
}
