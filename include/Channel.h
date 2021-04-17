//
// Created by gaojian on 2020/7/9.
//

#ifndef NETPP_CHANNEL_H
#define NETPP_CHANNEL_H

#include <memory>

namespace netpp {
class ByteArray;
namespace internal::handlers {
class TcpConnection;
}

/**
 * @brief A Channel represent a bridge to read/write TcpConnection's buffer.
 */
class Channel {
public:
	Channel(std::weak_ptr<internal::handlers::TcpConnection> connection, 
			std::weak_ptr<ByteArray> writeByteArray, std::weak_ptr<ByteArray> readByteArray);

	/**
	 * @brief Send byte array to socket
	 */
	void send();

	/**
	 * @brief Close a socket after write completed
	 */
	void close();
	
	/**
	 * @brief Write to byte array
	 * @return true		on write success
	 * @return false	if the connection is closing or closed
	 */
	bool writeInt8(int8_t value);
	bool writeInt16(int16_t value);
	bool writeInt32(int32_t value);
	bool writeInt64(int64_t value);
	bool writeUInt8(uint8_t value);
	bool writeUInt16(uint16_t value);
	bool writeUInt32(uint32_t value);
	bool writeUInt64(uint64_t value);
	bool writeFloat(float value);
	bool writeDouble(double value);
	bool writeString(const std::string &value);
	bool writeRaw(const char *data, std::size_t length);

	// TODO: support sendfile and mmap
	
	/**
	 * @brief The readable bytes in byte array
	 */
	[[nodiscard]] std::size_t availableRead() const;

	/**
	 * @brief Read from byte array
	 */
	int8_t retrieveInt8();
	int16_t retrieveInt16();
	int32_t retrieveInt32();
	int64_t retrieveInt64();
	uint8_t retrieveUInt8();
	uint16_t retrieveUInt16();
	uint32_t retrieveUInt32();
	uint64_t retrieveUInt64();
	float retrieveFloat();
	double retrieveDouble();
	std::string retrieveString(std::size_t length);
	std::size_t retrieveRaw(char *buffer, std::size_t length);

	/**
	 * @brief Get unique id for this channel
	 * @return -1 if the connection is closed
	 */
	int channelId();

private:
	/**
	 * @brief Get write array
	 * @return _writeArray	if connection is available and connection state is Established
	 * @return nullptr		on other conditions
	 */
	std::shared_ptr<ByteArray> writableArray();

	std::weak_ptr<internal::handlers::TcpConnection> _connection;
	std::weak_ptr<ByteArray> _writeArray;
	std::weak_ptr<ByteArray> _readArray;
};
}

#endif //NETPP_CHANNEL_H
