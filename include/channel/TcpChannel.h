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
 * @brief Provide only write methods to byte array
 */
class ChannelWriter {
public:
	/**
	 * @brief Create write on buffer
	 * @param writeBuffer The buffer bind to this writer
	 */
	explicit ChannelWriter(std::shared_ptr<ByteArray> writeBuffer);

	/**
	 * @brief Write an int8 to channel
	 * @param value Value to be write
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeInt8(int8_t value);

	/**
	 * @brief Write an int16 to channel
	 * @param value Value to be write, will automatic cast ending
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeInt16(int16_t value);

	/**
	 * @brief Write an int32 to channel
	 * @param value Value to be write, will automatic cast ending
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeInt32(int32_t value);

	/**
	 * @brief Write an int64 to channel
	 * @param value Value to be write, will automatic cast ending
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeInt64(int64_t value);

	/**
	 * @brief Write an uint8 to channel
	 * @param value Value to be write
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeUInt8(uint8_t value);

	/**
	 * @brief Write an uint16 to channel
	 * @param value Value to be write, will automatic cast ending
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeUInt16(uint16_t value);

	/**
	 * @brief Write an uint32 to channel
	 * @param value Value to be write, will automatic cast ending
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeUInt32(uint32_t value);

	/**
	 * @brief Write an uint64 to channel
	 * @param value Value to be write, will automatic cast ending
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeUInt64(uint64_t value);

	/**
	 * @brief Write a float to channel
	 * @param value Value to be write, ending not changed
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeFloat(float value);

	/**
	 * @brief Write a double to channel
	 * @param value Value to be write, ending not changed
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeDouble(double value);

	/**
	 * @brief Write a string to channel
	 * @param value Value to be write
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeString(const std::string &value);

	/**
	 * @brief Write raw data to channel
	 * @note writeRaw() writes data as it was, which means network-ending cast not performed
	 * @param data Value to be write
	 * @param length Length of data
	 * @return writer self, support chained calls
	 */
	ChannelWriter &writeRaw(const char *data, std::size_t length);

private:
	std::shared_ptr<ByteArray> m_writeBuffer;
};

/**
 * @brief Provide only read methods to byte array
 */
class ChannelReader {
public:
	/**
	 * @brief Create reader on buffer
	 * @param readBuffer The buffer bind to this writer
	 */
	explicit ChannelReader(std::shared_ptr<ByteArray> readBuffer);

	/**
	 * @brief Read an int8 from channel
	 * @return value retrieved
	 */
	std::optional<int8_t> retrieveInt8();

	/**
	 * @brief Read an int16 from channel, will cast to local-ending
	 * @return value retrieved
	 */
	std::optional<int16_t> retrieveInt16();

	/**
	 * @brief Read an int32 from channel, will cast to local-ending
	 * @return value retrieved
	 */
	std::optional<int32_t> retrieveInt32();

	/**
	 * @brief Read an int64 from channel, will cast to local-ending
	 * @return value retrieved
	 */
	std::optional<int64_t> retrieveInt64();

	/**
	 * @brief Read an uint8 from channel
	 * @return value retrieved
	 */
	std::optional<uint8_t> retrieveUInt8();

	/**
	 * @brief Read an uint16 from channel, will cast to local-ending
	 * @return value retrieved
	 */
	std::optional<uint16_t> retrieveUInt16();

	/**
	 * @brief Read an uint32 from channel, will cast to local-ending
	 * @return value retrieved
	 */
	std::optional<uint32_t> retrieveUInt32();

	/**
	 * @brief Read an uint64 from channel, will cast to local-ending
	 * @return value retrieved
	 */
	std::optional<uint64_t> retrieveUInt64();

	/**
	 * @brief Read a float from channel, ending not changed
	 * @return value retrieved
	 */
	std::optional<float> retrieveFloat();

	/**
	 * @brief Read a double from channel, ending not changed
	 * @return value retrieved
	 */
	std::optional<double> retrieveDouble();

	/**
	 * @brief Read a string from channel
	 * @param length The size of string
	 * @return value retrieved
	 */
	std::optional<std::string> retrieveString(std::size_t length);

	/**
	 * @brief Read raw data from channel, keep in network-ending
	 * @param buffer Buffer to store raw data
	 * @param length The size of string
	 * @return value retrieved
	 */
	std::optional<std::size_t> retrieveRaw(char *buffer, std::size_t length);

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
	/**
	 * @brief Create TcpChannel
	 * @param connection connection bind to this channel
	 */
	explicit TcpChannel(std::weak_ptr<internal::handlers::TcpConnection> connection);

	/**
	 * @brief TcpChannel's conversion method
	 * @return Conversion object
	 */
	std::unique_ptr<internal::buffer::ChannelBufferConversion> createBufferConvertor() override;

	/**
	 * @brief Write to Channel
	 * @return writer
	 */
	ChannelWriter writer();
	/**
	 * @brief Read from Channel
	 * @return reader
	 */
	ChannelReader reader();

	// TODO: support sendfile and mmap

private:
	std::shared_ptr<ByteArray> m_sendBuffer;
	std::shared_ptr<ByteArray> m_receiveArray;
};
}

#endif //NETPP_TCPCHANNEL_H
