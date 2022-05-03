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
	 * @brief Create a socket
	 * 
	 * @param addr	Address
	 * @throw SocketException on (EACCES EAFNOSUPPORT)
	 * @throw ResourceLimitException on (EMFILE ENFILE ENOBUFS ENOMEM)
	 */
	explicit Socket(const Address &addr);

	/**
	 * @brief Create a Socket object on existent fd
	 * 
	 * @param fd	File descriptor
	 * @param addr	Address
	 * @throw SocketException on fd is -1
	 */
	Socket(int fd, const Address &addr);

	Socket(Socket &&other) = delete;
	Socket(const Socket &) = delete;
	Socket &operator=(Socket &rh) = delete;
	~Socket();

	/**
	 * @brief Get socket fd, will not change once constructed
	 * @return
	 */
	[[nodiscard]] int fd() const noexcept { return m_socketFd; }

	/**
	 * @brief Bind address and start listen
	 * 
	 * @throw SocketException on (EADDRINUSE)
	 * @throw ResourceLimitException on (ENOMEM)
	 */
	void listen();

	/**
	 * @brief Accept a connect
	 * 
	 * @throw SocketException on (ECONNABORTED)
	 * @throw ResourceLimitException on (EMFILE ENFILE ENOBUFS ENOMEM EPERM)
	 */
	[[nodiscard]] std::unique_ptr<Socket> accept() const;

	/**
	 * @brief Connect to server
	 * 
	 * @throw SocketException on (EADDRINUSE EADDRNOTAVAIL ECONNREFUSED EINPROGRESS ENETUNREACH ETIMEDOUT)
	 */
	void connect();

	/**
	 * @brief Get socket error code
	 */
	[[nodiscard]] error::SocketError getError() const noexcept;
	[[nodiscard]] Address getAddr() const noexcept { return m_addr; };

	/**
	 * @brief Shutdown write side of socket
	 */
	void shutdownWrite() const noexcept;

private:
	Address m_addr;
	int m_socketFd;
};
}

#endif //NETPP_SOCKET_H
