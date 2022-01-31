//
// Created by gaojian on 2020/7/9.
//

#ifndef NETPP_CHANNEL_H
#define NETPP_CHANNEL_H

#include <memory>
#include <optional>

namespace netpp {
class ByteArray;
namespace internal::handlers {
class TcpConnection;
}

class ChannelWriter {
public:
	explicit ChannelWriter(std::shared_ptr<ByteArray> &&writeBuffer);
	/**
	 * @brief Write to byte array
	 */
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
	std::shared_ptr<ByteArray> m_writeBuffer;
};

class ChannelReader {
public:
	explicit ChannelReader(std::shared_ptr<ByteArray> &&readBuffer);
	/**
	 * @brief Read from byte array
	 */
	std::optional<int8_t> retrieveInt8();
	std::optional<int16_t> retrieveInt16();
	std::optional<int32_t> retrieveInt32();
	std::optional<int64_t> retrieveInt64();
	std::optional<uint8_t> retrieveUInt8();
	std::optional<uint16_t> retrieveUInt16();
	std::optional<uint32_t> retrieveUInt32();
	std::optional<uint64_t> retrieveUInt64();
	std::optional<float> retrieveFloat();
	std::optional<double> retrieveDouble();
	std::optional<std::string> retrieveString(std::size_t length);
	std::optional<std::size_t> retrieveRaw(char *buffer, std::size_t length);

	/**
	 * @brief The readable bytes in byte array
	 */
	[[nodiscard]] std::size_t readableBytes() const;

private:
	std::shared_ptr<ByteArray> m_readBuffer;
};
/**
 * @brief A Channel represent a bridge to read/write TcpConnection's buffer.
 */
class Channel {
public:
	Channel(std::weak_ptr<internal::handlers::TcpConnection> connection,
			std::weak_ptr<ByteArray> prependByteArray, std::weak_ptr<ByteArray> writeByteArray, std::weak_ptr<ByteArray> readByteArray);

	/**
	 * @brief Tell event loop data on this channel is ready to send, event loop will send it later,
	 */
	void send();

	/**
	 * @brief Close the channel
	 */
	void close();

	ChannelWriter prependWriter();
	ChannelWriter writer();
	ChannelReader reader();

	// TODO: support sendfile and mmap

	/**
	 * @brief Get unique id for this channel
	 * @return -1 if the connection was closed
	 */
	int channelId();

	/**
	 * @brief Write to this channel is effective only when connection was established,
	 * otherwise write to the channel will not send anything.
	 * @return true: connection was established
	 */
	[[nodiscard]] bool channelActive() const;

private:
	std::weak_ptr<internal::handlers::TcpConnection> _connection;
	std::weak_ptr<ByteArray> _prependArray;
	std::weak_ptr<ByteArray> _writeArray;
	std::weak_ptr<ByteArray> _readArray;
};
}

#endif //NETPP_CHANNEL_H
