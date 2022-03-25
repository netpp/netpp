//
// Created by gaojian on 2022/3/22.
//

#ifndef NETPP_TCPCHANNEL_H
#define NETPP_TCPCHANNEL_H

#include "Channel.h"
#include <optional>

namespace netpp {
class ByteArray;
namespace internal::buffer {
class TcpChannelConversion;
}
/**
 * @brief Write to byte array
 */
class ChannelWriter {
public:
	explicit ChannelWriter(std::shared_ptr<ByteArray> writeBuffer);

	ChannelWriter &writeInt8(int8_t value);
	ChannelWriter &writeInt16(int16_t value);
	ChannelWriter &writeInt32(int32_t value);
	ChannelWriter &writeInt64(int64_t value);
	ChannelWriter &writeUInt8(uint8_t value);
	ChannelWriter &writeUInt16(uint16_t value);
	ChannelWriter &writeUInt32(uint32_t value);
	ChannelWriter &writeUInt64(uint64_t value);
	ChannelWriter &writeFloat(float value);
	ChannelWriter &writeDouble(double value);
	ChannelWriter &writeString(const std::string &value);
	ChannelWriter &writeRaw(const char *data, std::size_t length);

private:
	std::shared_ptr <ByteArray> m_writeBuffer;
};

/**
 * @brief Read from byte array
 */
class ChannelReader {
public:
	explicit ChannelReader(std::shared_ptr <ByteArray> readBuffer);

	std::optional <int8_t> retrieveInt8();
	std::optional <int16_t> retrieveInt16();
	std::optional <int32_t> retrieveInt32();
	std::optional <int64_t> retrieveInt64();
	std::optional <uint8_t> retrieveUInt8();
	std::optional <uint16_t> retrieveUInt16();
	std::optional <uint32_t> retrieveUInt32();
	std::optional <uint64_t> retrieveUInt64();
	std::optional<float> retrieveFloat();
	std::optional<double> retrieveDouble();
	std::optional <std::string> retrieveString(std::size_t length);
	std::optional <std::size_t> retrieveRaw(char *buffer, std::size_t length);

	/**
	 * @brief The readable bytes in byte array
	 */
	[[nodiscard]] std::size_t readableBytes() const;

private:
	std::shared_ptr <ByteArray> m_readBuffer;
};

/**
 * @brief The normal tcp connection buffer
 */
class TcpChannel : public Channel {
	friend class internal::buffer::TcpChannelConversion;
public:
	using BufferConversion = internal::buffer::TcpChannelConversion;

	explicit TcpChannel(std::weak_ptr<internal::handlers::TcpConnection> connection);

	std::unique_ptr<internal::buffer::ChannelBufferConversion> createBufferConvertor() override;

	ChannelWriter writer();
	ChannelReader reader();

	// TODO: support sendfile and mmap

private:
	std::weak_ptr<internal::handlers::TcpConnection> _connection;
	std::shared_ptr<ByteArray> m_writeArray;
	std::shared_ptr<ByteArray> m_readArray;
};
}

#endif //NETPP_TCPCHANNEL_H
