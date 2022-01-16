#ifndef NETPP_SOCKETIO_H
#define NETPP_SOCKETIO_H

#include <memory>
#include "ByteArray.h"

struct iovec;

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

/**
 * @brief Convert ByteArray to ::iovec, adapt to readv()/writev(), used by SocketIO
 */
class ByteArray2IOVec {
public:
	ByteArray2IOVec();
	virtual ~ByteArray2IOVec();

	::iovec *iovec() { return m_vec; }
	[[nodiscard]] std::size_t iovenLength() const { return m_vecLen; }
	virtual void adjustByteArray(ByteArray::LengthType size) = 0;
	virtual ByteArray::LengthType availableBytes() = 0;

	ByteArray2IOVec(ByteArray2IOVec &) = delete;
	ByteArray2IOVec(ByteArray2IOVec &&) = delete;
	ByteArray2IOVec &operator =(ByteArray2IOVec &) = delete;
	ByteArray2IOVec &operator =(ByteArray2IOVec &&) = delete;

protected:
	::iovec *m_vec;
	std::size_t m_vecLen;
	std::shared_ptr<ByteArray> _buffer;	// byte array
};

/**
 * @brief Read from ByteArray as iovec
 * @note ByteArray's lock is acquired until destruction
 * 
 */
class ByteArrayIOVecReaderWithLock : public ByteArray2IOVec {
public:
	explicit ByteArrayIOVecReaderWithLock(std::shared_ptr<ByteArray> buffer);
	~ByteArrayIOVecReaderWithLock() override;

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
class ByteArrayIOVecWriterWithLock : public ByteArray2IOVec {
public:
	explicit ByteArrayIOVecWriterWithLock(std::shared_ptr<ByteArray> buffer);
	~ByteArrayIOVecWriterWithLock() override;

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
