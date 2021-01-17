//
// Created by gaojian on 2020/7/5.
//

#include "socket/Socket.h"
#include "Log.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <arpa/inet.h>
}

using std::make_unique;

namespace netpp::socket {
Socket::Socket(const Address &addr)
{
	if ((m_socketFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) == -1)
	{
		// TODO: handle socket error
		switch (errno)
		{
			case EACCES:
			case EAFNOSUPPORT:
			case EINVAL:
			case EMFILE:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
			case EPROTONOSUPPORT:
			default:
				break;
		}
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to create Socket: {}", ::strerror(errno));
	}
	SPDLOG_LOGGER_DEBUG(logger, "Socket fd: {}", m_socketFd);
	m_addr = addr;
}

Socket::Socket(int fd, const Address &addr)
	: m_socketFd{fd}, m_addr{addr}
{
	if (m_socketFd == -1)
		SPDLOG_LOGGER_WARN(logger, "Socket fd is -1");
}

Socket::~Socket()
{
	SPDLOG_LOGGER_TRACE(logger, "socket {} destructed", m_socketFd);
	if (m_socketFd != -1)
	{
		if (::close(m_socketFd) == -1)
		{
			SPDLOG_LOGGER_WARN(logger, "Close socket failed due to {}", m_socketFd, ::strerror(errno));
			// TODO: handle close error
			switch (errno)
			{
				case EBADF:
				case EINTR:
				case EIO:
				case ENOSPC:
				case EDQUOT:
				default:
					break;
			}
		}
	}
}

void Socket::listen()
{
	if (m_socketFd == -1)
	{
		SPDLOG_LOGGER_CRITICAL(logger, "Invalid fd -1");
		return;
	}
	const sockaddr_in *inetAddr = m_addr.sockAddrIn();
	if (::bind(m_socketFd, reinterpret_cast<const sockaddr *>(inetAddr), sizeof(::sockaddr_in)) == -1)
	{
		// TODO: handle bind error
		switch (errno)
		{
			case EACCES:
			case EADDRINUSE:
			case EBADF:
			case EINVAL:
			case ENOTSOCK:
			case EADDRNOTAVAIL:
			case EFAULT:
			case ELOOP:
			case ENAMETOOLONG:
			case ENOENT:
			case ENOMEM:
			case ENOTDIR:
			case EROFS:
			default:
				break;
		}
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to bind address: {}", ::strerror(errno));
		// FIXME: error code or exception
		throw std::runtime_error("Failed to bind address");
	}
	if (::listen(m_socketFd, 10) == -1)
	{
		// TODO: handle listen error
		switch (errno)
		{
			case EADDRINUSE:
			case EBADF:
			case ENOTSOCK:
			case EOPNOTSUPP:
			default:
				break;
		}
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to listen: {}", ::strerror(errno));
		// FIXME: error code or exception
		throw std::runtime_error("Failed to listen");
	}
	SPDLOG_LOGGER_TRACE(logger, "Start listen");
}

std::unique_ptr<Socket> Socket::accept() const
{
	if (m_socketFd == -1)
	{
		SPDLOG_LOGGER_ERROR(logger, "Invalid fd -1");
		return nullptr;
	}
	int newSocket;
	std::shared_ptr<::sockaddr_in> addr{std::make_shared<::sockaddr_in>()};
	socklen_t addrSize = sizeof(::sockaddr_in);
	std::memset(addr.get(), 0, addrSize);
	// TODO: handle accept error
	if ((newSocket = ::accept4(m_socketFd, reinterpret_cast<::sockaddr *>(addr.get()), &addrSize, SOCK_NONBLOCK)) == -1)
	{
		switch(errno)
		{
			case EAGAIN:
			case EBADF:
			case ECONNABORTED:
			case EFAULT:
			case EINTR:
			case EINVAL:
			case EMFILE:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
			case ENOTSOCK:
			case EOPNOTSUPP:
			case EPROTO:
			case EPERM:
			default:
				break;
		}
		SPDLOG_LOGGER_ERROR(logger, "Accept failed due to {}", ::strerror(errno));
		return nullptr;
	}
	else
	{
		SPDLOG_LOGGER_TRACE(logger, "Accepted new client with fd {}", newSocket);
		return make_unique<Socket>(newSocket, Address(addr));
	}
}

void Socket::connect()
{
	if (m_socketFd != -1)
	{
		sockaddr_in *inetAddr = m_addr.sockAddrIn();
		if (::connect(m_socketFd, reinterpret_cast<const ::sockaddr *>(inetAddr), sizeof(::sockaddr_in)) == -1)
		{
			// TODO: handle connect failed
			switch (errno)
			{
				case EACCES:
				case EPERM:
				case EADDRINUSE:
				case EADDRNOTAVAIL:
				case EAFNOSUPPORT:
				case EAGAIN:
				case EALREADY:
				case EBADF:
				case ECONNREFUSED:
				case EFAULT:
				case EINPROGRESS:
				case EINTR:
				case EISCONN:
				case ENETUNREACH:
				case ENOTSOCK:
				case EPROTOTYPE:
				case ETIMEDOUT:
				default:
					break;
			}
		}
	}
	else
		SPDLOG_LOGGER_CRITICAL(logger, "Invalid fd -1");
}

void Socket::shutdownWrite()
{
	SPDLOG_LOGGER_TRACE(logger, "Shut down write for socket {}", m_socketFd);
	if (::shutdown(m_socketFd, SHUT_WR) == -1)
	{
		SPDLOG_LOGGER_CRITICAL(logger, "Faild to shutdown {} due to {}", m_socketFd, ::strerror(errno));
		// TODO: handle shutdown failed
		switch (errno)
		{
			case EBADF:
			case EINVAL:
			case ENOTCONN:
			case ENOTSOCK:
			default:
				break;
		}
	}
}

SocketError Socket::getError() const
{
	SocketError socketError = SocketError::E_UNKOWN;
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if (::getsockopt(m_socketFd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == -1)
	{
		SPDLOG_LOGGER_CRITICAL(logger, "Faild to get socket options due to {}", m_socketFd, ::strerror(errno));
		// TODO: handle getsockopt failed
		switch (errno)
		{
			case EBADF:
			case EINVAL:
			case ENOPROTOOPT:
			case EACCES:
			case ENOBUFS:
			default:
				break;
		}
	}
	else
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
#define SOCKET_ERROR_DEF(error) case E##error: socketError = SocketError::E_##error;
#endif
#ifndef LAST_SOCKET_ERROR_DEF
#define LAST_SOCKET_ERROR_DEF(error) SOCKET_ERROR_DEF(error)
#endif
		default:
			socketError = SocketError::E_UNKOWN;
		}
	}
	return socketError;
}

// void Socket::close() const
// {
// 	SPDLOG_LOGGER_TRACE(logger, "Closing {}", m_socketFd);
// 	::close(m_socketFd);
// }

}
