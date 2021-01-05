//
// Created by gaojian on 2020/7/9.
//

#ifndef NETPP_CHANNEL_H
#define NETPP_CHANNEL_H

#include "ByteArray.h"

namespace netpp {
namespace handlers {
class TcpConnection;
}

class Channel {
public:
	Channel(std::weak_ptr<handlers::TcpConnection> connection, 
			std::weak_ptr<ByteArray> writeByteArray, std::weak_ptr<ByteArray> readByteArray)
		: _connection{connection}, _writeArray{writeByteArray}, _readArray{readByteArray}
	{}

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
	inline void writeInt8(int8_t value)		{ auto array = _writeArray.lock(); if (array) array->writeInt8(value); }
	inline void writeInt16(int16_t value)	{ auto array = _writeArray.lock(); if (array) array->writeInt16(value); }
	inline void writeInt32(int32_t value)	{ auto array = _writeArray.lock(); if (array) array->writeInt32(value); }
	inline void writeInt64(int64_t value)	{ auto array = _writeArray.lock(); if (array) array->writeInt64(value); }
	inline void writeUInt8(uint8_t value)	{ auto array = _writeArray.lock(); if (array) array->writeUInt8(value); }
	inline void writeUInt16(uint16_t value)	{ auto array = _writeArray.lock(); if (array) array->writeUInt16(value); }
	inline void writeUInt32(uint32_t value)	{ auto array = _writeArray.lock(); if (array) array->writeUInt32(value); }
	inline void writeUInt64(uint64_t value)	{ auto array = _writeArray.lock(); if (array) array->writeUInt64(value); }
	inline void writeFloat(float value)		{ auto array = _writeArray.lock(); if (array) array->writeFloat(value); }
	inline void writeDouble(double value)	{ auto array = _writeArray.lock(); if (array) array->writeDouble(value); }
	inline void writeString(std::string value)				{ auto array = _writeArray.lock(); if (array) array->writeString(std::move(value)); }
	inline void writeRaw(const char *data, std::size_t length)	{ auto array = _writeArray.lock(); if (array) array->writeRaw(data, length); }

	/**
	 * @brief The readable bytes in byte array
	 */
	inline std::size_t availableRead()	{ auto array = _readArray.lock(); if (array) return array->readableBytes(); else return 0; }

	/**
	 * @brief Read from byte array
	 */
	inline int8_t retrieveInt8()		{ auto array = _readArray.lock(); if (array) return array->retrieveInt8(); else return 0; }
	inline int16_t retrieveInt16()		{ auto array = _readArray.lock(); if (array) return array->retrieveInt16(); else return 0; }
	inline int32_t retrieveInt32()		{ auto array = _readArray.lock(); if (array) return array->retrieveInt32(); else return 0; }
	inline int64_t retrieveInt64()		{ auto array = _readArray.lock(); if (array) return array->retrieveInt64(); else return 0; }
	inline uint8_t retrieveUInt8()		{ auto array = _readArray.lock(); if (array) return array->retrieveUInt8(); else return 0; }
	inline uint16_t retrieveUInt16()	{ auto array = _readArray.lock(); if (array) return array->retrieveUInt16(); else return 0; }
	inline uint32_t retrieveUInt32()	{ auto array = _readArray.lock(); if (array) return array->retrieveUInt32(); else return 0; }
	inline uint64_t retrieveUInt64()	{ auto array = _readArray.lock(); if (array) return array->retrieveUInt64(); else return 0; }
	inline float retrieveFloat()		{ auto array = _readArray.lock(); if (array) return array->retrieveFloat(); else return 0; }
	inline double retrieveDouble()		{ auto array = _readArray.lock(); if (array) return array->retrieveDouble(); else return 0; }
	inline std::string retrieveString(std::size_t length)				{ auto array = _readArray.lock(); if (array) return array->retrieveString(length); else return ""; }
	inline std::size_t retrieveRaw(char *buffer, std::size_t length)	{ auto array = _readArray.lock(); if (array) return array->retrieveRaw(buffer, length); else return 0; }

private:
	std::weak_ptr<handlers::TcpConnection> _connection;
	std::weak_ptr<ByteArray> _writeArray;
	std::weak_ptr<ByteArray> _readArray;
};
}

#endif //NETPP_CHANNEL_H
