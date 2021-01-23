//
// Created by gaojian on 2020/7/5.
//

#include "socket/Socket.h"
#include "Log.h"
#include <cstring>
#include "stub/IO.h"
#include "stub/Socket.h"
#include "error/Exception.h"
#include "error/SocketError.h"

using std::make_unique;

namespace netpp::socket {
Socket::Socket(const Address &addr)
{
	m_socketFd = stub::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	m_addr = addr;
	SPDLOG_LOGGER_DEBUG(logger, "Socket fd: {}", m_socketFd);
}

Socket::Socket(int fd, const Address &addr)
	: m_socketFd{fd}, m_addr{addr}
{
	if (m_socketFd == -1)
		throw error::SocketException(EBADF);
}

Socket::~Socket()
{
	SPDLOG_LOGGER_DEBUG(logger, "socket {} destructed", m_socketFd);
	stub::close(m_socketFd);
}

void Socket::listen()
{
	const sockaddr_in *inetAddr = m_addr.sockAddrIn();
	stub::bind(m_socketFd, reinterpret_cast<const sockaddr *>(inetAddr), sizeof(::sockaddr_in));
	stub::listen(m_socketFd, 10);
	SPDLOG_LOGGER_TRACE(logger, "Start listen");
}

std::unique_ptr<Socket> Socket::accept() const
{
	std::shared_ptr<::sockaddr_in> addr{std::make_shared<::sockaddr_in>()};
	socklen_t addrSize = sizeof(::sockaddr_in);
	std::memset(addr.get(), 0, addrSize);
	int newSocket = stub::accept4(m_socketFd, reinterpret_cast<::sockaddr *>(addr.get()), &addrSize, SOCK_NONBLOCK);
	SPDLOG_LOGGER_TRACE(logger, "Accepted new client with fd {}", newSocket);
	return make_unique<Socket>(newSocket, Address(addr));
}

void Socket::connect()
{
	sockaddr_in *inetAddr = m_addr.sockAddrIn();
	stub::connect(m_socketFd, reinterpret_cast<const ::sockaddr *>(inetAddr), sizeof(::sockaddr_in));
}

void Socket::shutdownWrite() noexcept
{
	SPDLOG_LOGGER_TRACE(logger, "Shut down write for socket {}", m_socketFd);
	stub::shutdown(m_socketFd, SHUT_WR);
}

error::SocketError Socket::getError() const noexcept
{
	error::SocketError socketError = error::SocketError::E_UNKOWN;
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if (stub::getsockopt(m_socketFd, SOL_SOCKET, SO_ERROR, &optval, &optlen) != -1)
	{
		switch (optval)
		{
#ifdef SOCKET_ERROR_DEF
#undef SOCKET_ERROR_DEF
#endif
#ifdef LAST_SOCKET_ERROR_DEF
#undef LAST_SOCKET_ERROR_DEF
#endif
#ifndef SOCKET_ERROR_DEF
#define SOCKET_ERROR_DEF(error) case E##error: socketError = error::SocketError::E_##error;
#endif
#ifndef LAST_SOCKET_ERROR_DEF
#define LAST_SOCKET_ERROR_DEF(error) SOCKET_ERROR_DEF(error)
#endif
		default:
			socketError = error::SocketError::E_UNKOWN;
		}
	}
	return socketError;
}
}
