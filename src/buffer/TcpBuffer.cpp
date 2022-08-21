//
// Created by gaojian on 2022/3/22.
//

#include "buffer/TcpBuffer.h"
#include "buffer/ByteArray.h"
#include "buffer/BufferIOConversion.h"

namespace netpp {
TcpBuffer::~TcpBuffer() = default;

void TcpBuffer::addWriteBuffer(const ByteArray &buffer)
{
	m_sendBuffers.emplace_back(std::make_shared<ByteArray>(buffer));
}

ByteArray TcpBuffer::peekReadBuffer(ByteArray::LengthType size)
{
	return ByteArray(*m_receiveArray, size, false);
}

ByteArray TcpBuffer::readBuffer(ByteArray::LengthType size)
{
	return ByteArray(*m_receiveArray, size, true);
}

std::unique_ptr<BufferIOConversion> TcpBuffer::sendBufferForIO()
{
	auto conv = std::make_unique<SequentialByteArrayReaderWithLock>(std::move(m_sendBuffers));
	m_sendBuffers = std::vector<std::shared_ptr<ByteArray>>();
	return conv;
}

std::unique_ptr<BufferIOConversion> TcpBuffer::receiveBufferForIO()
{
	return std::make_unique<ByteArrayWriterWithLock>(m_receiveArray);
}

ByteArray::LengthType TcpBuffer::readableBytes() const
{
	return  m_receiveArray->readableBytes();
}
}
