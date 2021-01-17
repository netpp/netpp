//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_SOCKET_H
#define NETPP_SOCKET_H

#include "Address.h"
#include <memory>
#include "socket/SocketEnums.h"

namespace netpp {
class ByteArray;
}

namespace netpp::socket {

class Socket {
public:
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
	SocketError getError() const;
	inline Address getAddr() const { return m_addr; };

	/**
	 * @brief Shutdown write side of socket
	 */
	void shutdownWrite();

private:
	Address m_addr;
	int m_socketFd;
};
}

#endif //NETPP_SOCKET_H
