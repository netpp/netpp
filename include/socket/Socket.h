//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_SOCKET_H
#define NETPP_SOCKET_H

#include "Address.h"
#include <memory>

namespace netpp {
class ByteArray;
namespace error {
enum class SocketError;
}
}

namespace netpp::internal::socket {

class Socket {
public:
	/**
	 * @brief create a socket
	 * 
	 * @param addr	address
	 * @throw SocketException on @code EACCES EAFNOSUPPORT
	 * @throw ResourceLimitException on @code EMFILE ENFILE ENOBUFS ENOMEM
	 */
	explicit Socket(const Address &addr);

	/**
	 * @brief create a Socket object on existent fd
	 * 
	 * @param fd	file descriptor
	 * @param addr	address
	 * @throw SocketException on fd is -1
	 */
	Socket(int fd, const Address &addr);

	Socket(Socket &&other) = delete;
	Socket(const Socket &) = delete;
	Socket &operator=(Socket &rh) = delete;
	~Socket();

	inline int fd() const noexcept { return m_socketFd; }

	/**
	 * @brief Bind address and start listen
	 * 
	 * @throw SocketException on @code EADDRINUSE
	 * @throw ResourceLimitException on @code ENOMEM
	 */
	void listen();

	/**
	 * @brief Accept a connect
	 * 
	 * @throw SocketException on @code ECONNABORTED
	 * @throw ResourceLimitException on @code EMFILE ENFILE ENOBUFS ENOMEM EPERM
	 */
	std::unique_ptr<Socket> accept() const;

	/**
	 * @brief Connect to server
	 * 
	 * @throw SocketException on @code EADDRINUSE EADDRNOTAVAIL ECONNREFUSED EINPROGRESS ENETUNREACH ETIMEDOUT
	 */
	void connect();

	/**
	 * @brief Get socket error code
	 */
	error::SocketError getError() const noexcept;
	inline Address getAddr() const noexcept { return m_addr; };

	/**
	 * @brief Shutdown write side of socket
	 */
	void shutdownWrite() noexcept;

private:
	Address m_addr;
	int m_socketFd;
};
}

#endif //NETPP_SOCKET_H
