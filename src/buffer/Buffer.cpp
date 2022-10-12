//
// Created by gaojian on 2022/9/27.
//

#include <cstring>
#include "buffer/Buffer.h"
#include "support/Log.h"

namespace netpp {
Buffer::Buffer()
		: m_bufferCapacity{DefaultBufferSize}, m_start{0}, m_end{0}
{
	m_buffer = new char[DefaultBufferSize];
}

Buffer::Buffer(const Buffer &other)
		: m_bufferCapacity{other.m_bufferCapacity}, m_start{other.m_start}, m_end{other.m_end}
{
	m_buffer = new char[m_bufferCapacity];
	std::memcpy(m_buffer, other.m_buffer + other.m_start, other.readableBytes());
}

Buffer::Buffer(Buffer &&other) noexcept
		: m_bufferCapacity{other.m_bufferCapacity}, m_start{other.m_start}, m_end{other.m_end},
		  m_buffer{other.m_buffer}
{
	other.m_bufferCapacity = 0;
	other.m_start = 0;
	other.m_end = 0;
	other.m_buffer = nullptr;
}

Buffer::~Buffer()
{
	if (!m_buffer)
		delete []m_buffer;
}

void Buffer::write(const char *data, BufferLength length)
{
	LOG_TRACE("need to write {} bytes", length);

	if (m_bufferCapacity - m_end <= length)
		allocAtLeast(length);

	std::memcpy(m_buffer + m_end, data, length);
	m_end += length;
}

BufferLength Buffer::retrieve(char *buffer, BufferLength length)
{
	BufferLength copySize = readableBytes();
	if (copySize > length)
		copySize = length;
	std::memcpy(buffer, m_buffer + m_start, copySize);
	m_start += copySize;
	return copySize;
}

BufferLength Buffer::readableBytes() const
{
	return m_end - m_start;
}

BufferLength Buffer::writeableBytes() const
{
	return m_bufferCapacity - m_end;
}

void Buffer::allocAtLeast(BufferLength size)
{
	BufferLength availableSize = m_bufferCapacity - m_end + m_start;
	BufferLength currentReadableSize = readableBytes();
	char *newBuffer;
	if (size * 2 < availableSize)
	{
		// just move buffer data to head
		BufferLength copySize = m_start;
		BufferLength copyOffset = 0;
		while (copyOffset + m_start < m_end)
		{
			std::memcpy(m_buffer + copyOffset, m_buffer + m_start + copyOffset, copySize);
			if (copyOffset + m_start + copySize > m_end)
				copySize = m_end - copyOffset - m_start;
			copyOffset += copySize;
		}
		newBuffer = m_buffer;
	}
	else
	{
		// alloc more
		do {
			m_bufferCapacity *= 2;
		} while (m_bufferCapacity < size);
		newBuffer = new char[m_bufferCapacity];
		std::memcpy(newBuffer, m_buffer + m_start, currentReadableSize);
		delete []m_buffer;
	}
	m_buffer = newBuffer;
	m_start = 0;
	m_end = currentReadableSize;
}

char *Buffer::getRawPointer()
{
	return m_buffer + m_start;
}

void Buffer::readOutSize(BufferLength size)
{
	m_start += size;
}

void Buffer::writeInSize(BufferLength size)
{
	m_end += size;
}
} // netpp
