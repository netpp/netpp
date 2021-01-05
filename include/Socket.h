//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_SOCKET_H
#define NETPP_SOCKET_H

#include "Address.h"
#include <memory>

namespace netpp {
class ByteArray;
}

namespace netpp {
class Socket {
public:
	Socket();
	explicit Socket(const Address &addr);
	Socket(int fd, const Address &addr);

	Socket(Socket &&other) = delete;
	Socket(const Socket &) = delete;
	Socket &operator=(Socket &rh) = delete;
	~Socket();

	inline int fd() const { return m_socketFd; }

	/**
	 * @brief Bind address and start listen
	 */
	void listen();

	/**
	 * @brief Accept a connect
	 */
	std::unique_ptr<Socket> accept() const;

	/**
	 * @brief Connect to server
	 */
	void connect();

	/**
	 * @brief Get socket error code
	 */
	int getError() const;
	inline Address getAddr() const { return m_addr; };

	/**
	 * @brief Shutdown write side of socket
	 */
	void shutdownWrite();

private:
	Address m_addr;
	int m_socketFd;
};

/**
 * @brief Read/Write on socket from/into ByteArray
 */
class SocketIO {
public:
	static void read(const Socket *socket, std::shared_ptr<ByteArray> byteArray);
	static void write(const Socket *socket, std::shared_ptr<ByteArray> byteArray);
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

#endif //NETPP_SOCKET_H
