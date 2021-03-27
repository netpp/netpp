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
 * 
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
	 */
	void writeInt8(int8_t value);
	void writeInt16(int16_t value);
	void writeInt32(int32_t value);
	void writeInt64(int64_t value);
	void writeUInt8(uint8_t value);
	void writeUInt16(uint16_t value);
	void writeUInt32(uint32_t value);
	void writeUInt64(uint64_t value);
	void writeFloat(float value);
	void writeDouble(double value);
	void writeString(std::string value);
	void writeRaw(const char *data, std::size_t length);

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

private:
	std::weak_ptr<internal::handlers::TcpConnection> _connection;
	std::weak_ptr<ByteArray> _writeArray;
	std::weak_ptr<ByteArray> _readArray;
};
}

#endif //NETPP_CHANNEL_H
