//
// Created by gaojian on 2022/9/27.
//

#include <cstring>
#include "buffer/Buffer.h"
#include "support/Log.h"

namespace netpp {
Buffer::Buffer()
		: m_bufferSize{DefaultBufferSize}, m_start{0}, m_end{0}
{
	m_buffer = new char[DefaultBufferSize];
}

Buffer::Buffer(const Buffer &other)
		: m_bufferSize{other.m_bufferSize}, m_start{other.m_start}, m_end{other.m_end}
{
	m_buffer = new char[m_bufferSize];
	std::memcpy(m_buffer, other.m_buffer + other.m_start, other.readableBytes());
}

Buffer::Buffer(Buffer &&other) noexcept
		: m_bufferSize{other.m_bufferSize}, m_start{other.m_start}, m_end{other.m_end},
		  m_buffer{other.m_buffer}
{
	other.m_bufferSize = 0;
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

	if (m_bufferSize - m_end <= length)
		allocAtLeast(length);

	std::memcpy(m_buffer + m_start, data, length);
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
	return m_bufferSize - m_end;
}

void Buffer::allocAtLeast(BufferLength size)
{
	do {
		m_bufferSize *= 2;
	} while (m_bufferSize < size);
	BufferLength currentReadableSize = readableBytes();
	char *newBuffer = new char[m_bufferSize];
	std::memcpy(newBuffer, m_buffer + m_start, currentReadableSize);
	delete []m_buffer;
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
