//
// Created by 17271 on 2022/6/8.
//

#include "buffer/BufferIOConversion.h"
extern "C" {
#include <sys/uio.h>
}

namespace netpp {
BufferIOConversion::BufferIOConversion()
		: m_vec{nullptr}, m_vecLen{0}
{
}

BufferIOConversion::~BufferIOConversion()
{
	delete[] m_vec;
}

ByteArrayReaderWithLock::ByteArrayReaderWithLock(std::shared_ptr <ByteArray> buffer)
		: m_buffer{std::move(buffer)}, m_lck{m_buffer->m_bufferMutex}
{
	m_vecLen = 0;
	auto *vecNodes = new ::iovec[m_buffer->m_writeNode - m_buffer->m_readNode + 1];

	auto end = m_buffer->m_writeNode + 1;
	for (auto it = m_buffer->m_readNode; it != end; ++it)
	{
		if (it->end - it->start != 0)        // this node is empty
		{
			vecNodes[m_vecLen].iov_base = it->buffer + it->start;
			vecNodes[m_vecLen].iov_len = it->end - it->start;
			++m_vecLen;
		}
	}
	if (m_vecLen != 0)
		m_vec = vecNodes;
}

ByteArrayReaderWithLock::~ByteArrayReaderWithLock() = default;

void ByteArrayReaderWithLock::adjustByteArray(ByteArray::LengthType size)
{
	if (size >= m_buffer->m_availableSizeToRead)    // if size is larger than this ByteArray, just read node to end
	{
		m_buffer->m_readNode = m_buffer->m_writeNode;
		m_buffer->m_availableSizeToRead = 0;
		auto node = *m_buffer->m_readNode;
		m_buffer->m_readNode->start = m_buffer->m_readNode->end;
	}
	else
	{
		m_buffer->m_availableSizeToRead -= size;
		ByteArray::CowBuffer::NodeContainerIndexer moveForward = size / ByteArray::BufferNodeSize;
		ByteArray::CowBuffer::NodeContainerIndexer left = size % ByteArray::BufferNodeSize;
		moveForward += (moveForward != 0 && left != 0);
		m_buffer->m_readNode += moveForward;
		m_buffer->m_readNode->start += left;
	}
	m_buffer->unlockedMoveBufferHead();
}

ByteArray::LengthType ByteArrayReaderWithLock::availableBytes()
{
	return m_buffer->m_availableSizeToRead;
}

ByteArrayWriterWithLock::ByteArrayWriterWithLock(std::shared_ptr <ByteArray> buffer)
		: m_buffer(std::move(buffer)), m_lck{m_buffer->m_bufferMutex}
{
	uint64_t bytes = m_buffer->m_availableSizeToWrite;
	m_vecLen = bytes / ByteArray::BufferNodeSize + (bytes % ByteArray::BufferNodeSize != 0);
	if (m_vecLen > 0)
	{
		m_vec = new ::iovec[m_vecLen];
		int i = 0;
		for (auto it = m_buffer->m_writeNode; it != m_buffer->m_nodes->end(); ++it)
		{
			m_vec[i].iov_base = it->buffer + it->end;
			m_vec[i].iov_len = ByteArray::BufferNodeSize - it->end;
			++i;
		}
	}
}

ByteArrayWriterWithLock::~ByteArrayWriterWithLock() = default;

void ByteArrayWriterWithLock::adjustByteArray(ByteArray::LengthType size)
{
	// if need to alloc more
	if (m_buffer->m_availableSizeToWrite <= size)
		m_buffer->unlockedAllocIfNotEnough(size);
	m_buffer->m_availableSizeToWrite -= size;
	m_buffer->m_availableSizeToRead += size;

	for (auto it = m_buffer->m_writeNode; it != m_buffer->m_nodes->end(); ++it)
	{
		it->end = (it->end + size >= ByteArray::BufferNodeSize) ? ByteArray::BufferNodeSize : (it->end + size);
		size -= (ByteArray::BufferNodeSize - it->end);
		if (m_buffer->m_writeNode + 1 != m_buffer->m_nodes->end())
			++m_buffer->m_writeNode;
	}
}

ByteArray::LengthType ByteArrayWriterWithLock::availableBytes()
{
	return m_buffer->m_availableSizeToWrite;
}

SequentialByteArrayReaderWithLock::SequentialByteArrayReaderWithLock(
		std::initializer_list <std::shared_ptr<ByteArray>> buffers)
		: SequentialByteArrayReaderWithLock(std::vector(buffers))
{}

SequentialByteArrayReaderWithLock::SequentialByteArrayReaderWithLock(std::vector <std::shared_ptr<ByteArray>> &&buffers)
		: m_buffers(buffers)
{
	ByteArray::CowBuffer::NodeContainerIndexer nodeCount = 0;
	for (auto &b: m_buffers)
	{
		m_lck.emplace_back(std::make_unique < std::lock_guard < ByteArrayMutex >> (b->m_bufferMutex));
		ByteArray::CowBuffer::NodeContainerIndexer size = b->m_nodes->size();
		if (size > 0)
			nodeCount += size;
	}

	if (nodeCount != 0)
	{
		m_vecLen = 0;
		auto *vecNodes = new ::iovec[nodeCount];
		for (auto &b: m_buffers)
		{
			auto end = b->m_writeNode + 1;
			for (auto it = b->m_readNode; it != end; ++it)
			{
				if (it->end - it->start != 0)        // this node is empty
				{
					vecNodes[m_vecLen].iov_base = it->buffer + it->start;
					vecNodes[m_vecLen].iov_len = it->end - it->start;
					++m_vecLen;
				}
			}
		}
		if (m_vecLen != 0)
			m_vec = vecNodes;
	}
}

SequentialByteArrayReaderWithLock::~SequentialByteArrayReaderWithLock() = default;

void SequentialByteArrayReaderWithLock::adjustByteArray(ByteArray::LengthType size)
{
	for (auto &b: m_buffers)
	{
		if (size >= b->m_availableSizeToRead)    // if size is larger than this ByteArray, just read node to end
		{
			size -= b->m_availableSizeToRead;
			b->m_readNode = b->m_writeNode;
			b->m_availableSizeToRead = 0;
			b->m_readNode->start = b->m_readNode->end;
		}
		else
		{
			b->m_availableSizeToRead -= size;
			ByteArray::CowBuffer::NodeContainerIndexer moveForward = size / ByteArray::BufferNodeSize;
			ByteArray::CowBuffer::NodeContainerIndexer left = size % ByteArray::BufferNodeSize;
			moveForward += (moveForward != 0 && left != 0);
			size = 0;
			b->m_readNode += moveForward;
			b->m_readNode->start += left;
		}
		b->unlockedMoveBufferHead();
		if (size <= 0)
			break;
	}
}

ByteArray::LengthType SequentialByteArrayReaderWithLock::availableBytes()
{
	ByteArray::LengthType length = 0;
	for (auto &b: m_buffers)
		length += b->m_availableSizeToRead;
	return length;
}
}
