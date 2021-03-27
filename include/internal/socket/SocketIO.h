#ifndef NETPP_SOCKETIO_H
#define NETPP_SOCKETIO_H

#include <memory>
#include "ByteArray.h"

struct msghdr;

namespace netpp::internal::socket {
class Socket;

/**
 * @brief Read/Write on socket from/into ByteArray
 */
namespace SocketIO {
	/**
	 * @brief read from socket
	 * 
	 * @param socket	a socket object
	 * @param byteArray	the buffer
	 * @throw ResourceLimitException on (ENOMEM)
	 * @throw SocketException on (ECONNREFUSED ENOTCONN)
	 */
	void read(const Socket *socket, std::shared_ptr<ByteArray> byteArray);

	/**
	 * @brief write to a socket
	 * 
	 * @param socket	a socket object
	 * @param byteArray	the buffer
	 * @return true		write all
	 * @return false	not write all, more data in buffer to write
	 * @throw SocketException on (ECONNRESET EDESTADDRREQ EISCONN EMSGSIZE ENOTCONN EPIPE)
	 * @throw ResourceLimitException on (ENOMEM)
	 */
	bool write(const Socket *socket, std::shared_ptr<ByteArray> byteArray);
};

// TODO: just contain ::iovec in ByteArray for better performance??
/**
 * @brief Convert ByteArray to ::msghdr, adapt to readv()/writev(), used by SocketIO
 */
class ByteArray2Msghdr {
public:
	ByteArray2Msghdr();
	virtual ~ByteArray2Msghdr();

	// TODO: return ::iovec *, provide more scalability
	::msghdr *msghdr() { return msg; }
	virtual void adjustByteArray(ByteArray::LengthType size) = 0;
	virtual ByteArray::LengthType availableBytes() = 0;

	ByteArray2Msghdr(ByteArray2Msghdr &) = delete;
	ByteArray2Msghdr(ByteArray2Msghdr &&) = delete;
	ByteArray2Msghdr &operator =(ByteArray2Msghdr &) = delete;
	ByteArray2Msghdr &operator =(ByteArray2Msghdr &&) = delete;

protected:
	::msghdr *msg;
	std::shared_ptr<ByteArray> _buffer;	// byte array
};

/**
 * @brief Read from ByteArray as iovec
 * @note ByteArray's lock is acquired until destruction
 * 
 */
class ByteArrayReaderWithLock : public ByteArray2Msghdr {
public:
	explicit ByteArrayReaderWithLock(std::shared_ptr<ByteArray> buffer);
	~ByteArrayReaderWithLock() override;

	/**
	 * @brief Write n bytes into ByteArray
	 * @param size the return value of writev()
	 */
	void adjustByteArray(ByteArray::LengthType size) override;

	/**
	 * @brief Get readable bytes in ByteArray
	 * 
	 * @return ByteArray::LengthType	readable bytes
	 */
	ByteArray::LengthType availableBytes() override;
};

/**
 * @brief Write to ByteArray as iovec
 * @note ByteArray's lock is acquired until destruction
 * 
 */
class ByteArrayWriterWithLock : public ByteArray2Msghdr {
public:
	explicit ByteArrayWriterWithLock(std::shared_ptr<ByteArray> buffer);
	~ByteArrayWriterWithLock() override;

	/**
	 * @brief Read n bytes from ByteArray
	 * @param size the return value of readv()
	 */
	void adjustByteArray(ByteArray::LengthType size) override;

	/**
	 * @brief Get writeable bytes in ByteArray
	 * 
	 * @return ByteArray::LengthType	writeable bytes
	 */
	ByteArray::LengthType availableBytes() override;
};
}

#endif
