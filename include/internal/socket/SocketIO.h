#ifndef NETPP_SOCKETIO_H
#define NETPP_SOCKETIO_H

#include <memory>
#include <vector>
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
	bool write(const Socket *socket, std::vector<std::shared_ptr<ByteArray>> &&buffers);
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
};

/**
 * @brief Convert a ByteArray to iovec, for read data from iovec
 * @note ByteArray's lock is acquired until destruction
 * 
 */
class ByteArrayReaderWithLock : public ByteArray2IOVec {
public:
	explicit ByteArrayReaderWithLock(std::shared_ptr<ByteArray> buffer);
	~ByteArrayReaderWithLock() override;

	/**
	 * @brief After read 'size' data from ByteArray, move read node backwards
	 * @param size The size expect to move
	 */
	void adjustByteArray(ByteArray::LengthType size) override;

	/**
	 * @brief Get readable bytes in ByteArray
	 * 
	 * @return ByteArray::LengthType	readable bytes
	 */
	ByteArray::LengthType availableBytes() override;

private:
	std::shared_ptr<ByteArray> m_buffer;
};

/**
 * @brief Convert several ByteArrays to one iovec, improve the flexibility of reading,
 * for example, we can prepend/append anything on an existent buffer, just create an
 * other ByteArray
 */
class SequentialByteArrayReaderWithLock : public ByteArray2IOVec {
public:
	explicit SequentialByteArrayReaderWithLock(std::vector<std::shared_ptr<ByteArray>> &&buffers);
	~SequentialByteArrayReaderWithLock() noexcept override;

	/**
	 * @brief After read 'size' data from ByteArray, move read node backwards
	 * @param size The size expect to move
	 */
	void adjustByteArray(ByteArray::LengthType size) override;

	/**
	 * @brief Get readable bytes in ByteArrays
	 *
	 * @return ByteArray::LengthType	readable bytes
	 */
	ByteArray::LengthType availableBytes() override;

private:
	std::vector<std::shared_ptr<ByteArray>> m_buffers;
};

/**
 * @brief Convert ByteArray to iovec, for write data to iovec
 * @note ByteArray's lock is acquired until destruction
 * 
 */
class ByteArrayWriterWithLock : public ByteArray2IOVec {
public:
	explicit ByteArrayWriterWithLock(std::shared_ptr<ByteArray> buffer);
	~ByteArrayWriterWithLock() override;

	/**
	 * @brief After write 'size' data to ByteArray, move write node backwards
	 * @param size The size expect to move
	 */
	void adjustByteArray(ByteArray::LengthType size) override;

	/**
	 * @brief Get writeable bytes in ByteArray
	 * 
	 * @return ByteArray::LengthType	writeable bytes
	 */
	ByteArray::LengthType availableBytes() override;

private:
	std::shared_ptr<ByteArray> m_buffer;
};
}

#endif
