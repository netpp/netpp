#ifndef NETPP_SOCKETIO_H
#define NETPP_SOCKETIO_H

#include <memory>

struct iovec;
namespace netpp {
class ByteArray;
}

namespace netpp::socket {
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
	 * @throw ResourceLimitException on @code ENOMEM
	 * @throw SocketException on @code ECONNREFUSED ENOTCONN
	 */
	void read(const Socket *socket, std::shared_ptr<ByteArray> byteArray);

	/**
	 * @brief write to a socket
	 * 
	 * @param socket	a socket object
	 * @param byteArray	the buffer
	 * @return true		write all
	 * @return false	not write all, more data in buffer to write
	 * @throw SocketException on @code ECONNRESET EDESTADDRREQ EISCONN EMSGSIZE ENOTCONN EPIPE
	 * @throw ResourceLimitException on @code ENOMEM
	 */
	bool write(const Socket *socket, std::shared_ptr<ByteArray> byteArray);
};

/**
 * @brief Convert ByteArray to ::iovec, adapte to readv()/writev(), used by SocketIO
 */
class ByteArray2IOVector {
public:
	ByteArray2IOVector();
	virtual ~ByteArray2IOVector();

	inline int count() { return m_count; }
	inline ::iovec *vec() { return m_vec; }
	virtual void adjustByteArray(std::size_t size) = 0;

	ByteArray2IOVector(ByteArray2IOVector &) = delete;
	ByteArray2IOVector(ByteArray2IOVector &&) = delete;
	ByteArray2IOVector &operator =(ByteArray2IOVector &) = delete;

protected:
	::iovec *m_vec;		// iovec array
	int m_count;		// iovec count
	std::shared_ptr<ByteArray> _buffer;	// byte array
};

/**
 * @brief Read from ByteArray as iovec, will LOCK buffer until destruction
 * 
 */
class ByteArrayIOVectorReaderWithLock : public ByteArray2IOVector {
public:
	explicit ByteArrayIOVectorReaderWithLock(std::shared_ptr<ByteArray> buffer);
	virtual ~ByteArrayIOVectorReaderWithLock() override;

	/**
	 * @brief Write n bytes into ByteArray
	 * @param size the return value of writev()
	 */
	virtual void adjustByteArray(std::size_t size) override;
};

/**
 * @brief Write to ByteArray as iovec, will LOCK buffer until destruction
 * 
 */
class ByteArrayIOVectorWriterWithLock : public ByteArray2IOVector {
public:
	explicit ByteArrayIOVectorWriterWithLock(std::shared_ptr<ByteArray> buffer);
	virtual ~ByteArrayIOVectorWriterWithLock() override;

	/**
	 * @brief Read n bytes from ByteArray
	 * @param size the return value of readv()
	 */
	virtual void adjustByteArray(std::size_t size) override;
};
}

#endif