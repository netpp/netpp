//
// Created by gaojian on 2022/3/22.
//

#include "channel/TcpChannel.h"
#include "ByteArray.h"
#include "internal/buffer/ChannelBufferConversion.h"

namespace netpp {
ChannelWriter::ChannelWriter(std::shared_ptr<ByteArray> writeBuffer)
		: m_writeBuffer{std::move(writeBuffer)}
{}

ChannelWriter &ChannelWriter::writeInt8(int8_t value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeInt8(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeInt16(int16_t value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeInt16(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeInt32(int32_t value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeInt32(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeInt64(int64_t value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeInt64(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeUInt8(uint8_t value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeUInt8(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeUInt16(uint16_t value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeUInt16(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeUInt32(uint32_t value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeUInt32(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeUInt64(uint64_t value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeUInt64(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeFloat(float value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeFloat(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeDouble(double value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeDouble(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeString(const std::string &value)
{
	if (m_writeBuffer)
		m_writeBuffer->writeString(value);
	return *this;
}

ChannelWriter &ChannelWriter::writeRaw(const char *data, std::size_t length)
{
	if (m_writeBuffer)
		m_writeBuffer->writeRaw(data, length);
	return *this;
}

ChannelReader::ChannelReader(std::shared_ptr<ByteArray> readBuffer)
		: m_readBuffer{std::move(readBuffer)}
{}

std::optional<int8_t> ChannelReader::retrieveInt8()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(int8_t))
		return m_readBuffer->retrieveInt8();
	return std::nullopt;
}

std::optional<int16_t> ChannelReader::retrieveInt16()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(int16_t))
		return m_readBuffer->retrieveInt16();
	return std::nullopt;
}

std::optional<int32_t> ChannelReader::retrieveInt32()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(int32_t))
		return m_readBuffer->retrieveInt32();
	return std::nullopt;
}

std::optional<int64_t> ChannelReader::retrieveInt64()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(int64_t))
		return m_readBuffer->retrieveInt64();
	return std::nullopt;
}

std::optional<uint8_t> ChannelReader::retrieveUInt8()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(uint8_t))
		return m_readBuffer->retrieveUInt8();
	return std::nullopt;
}

std::optional<uint16_t> ChannelReader::retrieveUInt16()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(uint16_t))
		return m_readBuffer->retrieveUInt16();
	return std::nullopt;
}

std::optional<uint32_t> ChannelReader::retrieveUInt32()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(uint32_t))
		return m_readBuffer->retrieveUInt32();
	return std::nullopt;
}

std::optional<uint64_t> ChannelReader::retrieveUInt64()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(uint64_t))
		return m_readBuffer->retrieveUInt64();
	return std::nullopt;
}

std::optional<float> ChannelReader::retrieveFloat()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(float))
		return m_readBuffer->retrieveFloat();
	return std::nullopt;
}

std::optional<double> ChannelReader::retrieveDouble()
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= sizeof(double))
		return m_readBuffer->retrieveDouble();
	return std::nullopt;
}

std::optional<std::string> ChannelReader::retrieveString(std::size_t length)
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= length)
		return m_readBuffer->retrieveString(length);
	return std::nullopt;
}

std::optional<std::size_t> ChannelReader::retrieveRaw(char *buffer, std::size_t length)
{
	if (m_readBuffer && m_readBuffer->readableBytes() >= length)
		return m_readBuffer->retrieveRaw(buffer, length);
	return std::nullopt;
}

std::size_t ChannelReader::readableBytes() const
{
	if (m_readBuffer)
		return m_readBuffer->readableBytes();
	return 0;
}

TcpChannel::TcpChannel(std::weak_ptr<internal::handlers::TcpConnection> connection)
		: Channel(std::move(connection)),
		  m_sendBuffer{std::make_shared<ByteArray>()}, m_receiveArray{std::make_shared<ByteArray>()}
{}

std::unique_ptr<internal::buffer::ChannelBufferConversion> TcpChannel::createBufferConvertor()
{
	return std::make_unique<internal::buffer::TcpChannelConversion>();
}

ChannelWriter TcpChannel::writer()
{
	return ChannelWriter(m_sendBuffer);
}

ChannelReader TcpChannel::reader()
{
	return ChannelReader(m_receiveArray);
}
}
