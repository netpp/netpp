//
// Created by gaojian on 2020/7/5.
//

#include "socket/Socket.h"
#include "support/Log.h"
#include <cstring>
#include "stub/IO.h"
#include "stub/Socket.h"
#include "error/Exception.h"
#include "error/SocketError.h"

using std::make_unique;

namespace netpp::internal::socket {
Socket::Socket(const Address &addr)
{
	m_socketFd = stub::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	m_addr = addr;
	LOG_DEBUG("Socket fd: {}", m_socketFd);
}

Socket::Socket(int fd, const Address &addr)
	: m_addr{addr}, m_socketFd{fd}
{
	if (m_socketFd == -1)
		throw error::SocketException(EBADF);
}

Socket::~Socket()
{
	LOG_DEBUG("socket {} destructed", m_socketFd);
	stub::close(m_socketFd);
}

void Socket::listen()
{
	const sockaddr_in *inetAddr = m_addr.sockAddrIn();
	stub::bind(m_socketFd, reinterpret_cast<const sockaddr *>(inetAddr), sizeof(::sockaddr_in));
	stub::listen(m_socketFd, 10);
	LOG_TRACE("Start listen");
}

std::unique_ptr<Socket> Socket::accept() const
{
	std::shared_ptr<::sockaddr_in> addr{std::make_shared<::sockaddr_in>()};
	socklen_t addrSize = sizeof(::sockaddr_in);
	std::memset(addr.get(), 0, addrSize);
	int newSocket = stub::accept4(m_socketFd, reinterpret_cast<::sockaddr *>(addr.get()), &addrSize, SOCK_NONBLOCK | SOCK_CLOEXEC);
	LOG_TRACE("Accepted new client with fd {}", newSocket);
	return make_unique<Socket>(newSocket, Address(addr));
}

void Socket::connect()
{
	sockaddr_in *inetAddr = m_addr.sockAddrIn();
	stub::connect(m_socketFd, reinterpret_cast<const ::sockaddr *>(inetAddr), sizeof(::sockaddr_in));
}

void Socket::shutdownWrite() noexcept
{
	LOG_TRACE("Shut down write for socket {}", m_socketFd);
	stub::shutdown(m_socketFd, SHUT_WR);
}

error::SocketError Socket::getError() const noexcept
{
#ifdef SOCKET_ERROR_DEF
#undef SOCKET_ERROR_DEF
#endif
#ifdef SOCKET_ERROR_NONE_LINUX
#undef SOCKET_ERROR_NONE_LINUX
#endif
#ifdef LAST_SOCKET_ERROR_DEF
#undef LAST_SOCKET_ERROR_DEF
#endif
#ifndef SOCKET_ERROR_DEF
#define SOCKET_ERROR_DEF(code) case E##code: socketError = error::SocketError::E_##code;	break;
#endif
#ifndef SOCKET_ERROR_NONE_LINUX
#define SOCKET_ERROR_NONE_LINUX(code) ;
#endif
#ifndef LAST_SOCKET_ERROR_DEF
#define LAST_SOCKET_ERROR_DEF(code) SOCKET_ERROR_DEF(code)
#endif
	error::SocketError socketError = error::SocketError::E_UNKOWN;
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if (stub::getsockopt(m_socketFd, SOL_SOCKET, SO_ERROR, &optval, &optlen) != -1)
	{
		switch (optval)
		{
			case 0: socketError = error::SocketError::E_NOERROR;	break;
#include "error/SocketError.def"
		}
	}
	return socketError;
}
}
