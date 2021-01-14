//
// Created by gaojian on 2020/7/5.
//

#include "support/Socket.h"
#include "Log.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
}

using std::make_unique;

namespace netpp::support {
Socket::Socket()
{
	if ((m_socketFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP)) == -1)
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to create Socket: {}", std::string(strerror(errno)));
	SPDLOG_LOGGER_DEBUG(logger, "Socket fd: {}", m_socketFd);
}

Socket::Socket(const Address &addr)
	: Socket()
{
	m_addr = addr;
}

Socket::Socket(int fd, const Address &addr)
	: m_socketFd{fd}, m_addr{addr}
{
	if (m_socketFd == -1)
		SPDLOG_LOGGER_ERROR(logger, "Socket fd is -1");
}

Socket::~Socket()
{
	SPDLOG_LOGGER_TRACE(logger, "socket {} destructed", m_socketFd);
	if (m_socketFd != -1)
		::close(m_socketFd);
}

void Socket::listen()
{
	if (m_socketFd == -1)
	{
		SPDLOG_LOGGER_CRITICAL(logger, "Invalid fd -1");
		return;
	}
	const sockaddr_in *inetAddr = m_addr.sockAddrIn();
	if (::bind(m_socketFd, reinterpret_cast<const sockaddr *>(inetAddr), sizeof(*inetAddr)) == -1)
	{
		std::string errnoStr = strerror(errno);
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to bind address: {}", errnoStr);
		throw std::runtime_error("Failed to bind address " + errnoStr);
	}
	if (::listen(m_socketFd, 10) == -1)
	{
		std::string errnoStr = strerror(errno);
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to listen: {}", errnoStr);
		throw std::runtime_error("Failed to listen: " + errnoStr);
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
	sockaddr_in inetAddr{0};
	socklen_t addrSize = sizeof(inetAddr);
	// TODO: handle accept error
	if ((newSocket = ::accept4(m_socketFd, reinterpret_cast<sockaddr *>(&inetAddr), &addrSize, SOCK_NONBLOCK)) == -1)
	{
		SPDLOG_LOGGER_ERROR(logger, "Failed to accept: {}", std::string(strerror(errno)));
		return nullptr;
	}
	SPDLOG_LOGGER_TRACE(logger, "Accepted new client with fd {}", newSocket);
	return make_unique<Socket>(newSocket, Address(inetAddr));
}

void Socket::connect()
{
	if (m_socketFd == -1)
	{
		SPDLOG_LOGGER_CRITICAL(logger, "Invalid fd -1");
		return;
	}
	const sockaddr_in *inetAddr = m_addr.sockAddrIn();
	// FIXME: handle connect failed
	::connect(m_socketFd, reinterpret_cast<const sockaddr *>(inetAddr), sizeof(*inetAddr));
}

void Socket::shutdownWrite()
{
	if (m_socketFd != -1)
	{
		::shutdown(m_socketFd, SHUT_WR);
		SPDLOG_LOGGER_TRACE(logger, "Shut down write for socket {}", m_socketFd);
	}
	else
		SPDLOG_LOGGER_CRITICAL(logger, "Invalid fd -1");
}

int Socket::getError() const
{
	int optval;
	socklen_t optlen = static_cast<socklen_t>(sizeof optval);
	if (::getsockopt(m_socketFd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
		return errno;
	else
		return optval;
}

// void Socket::close() const
// {
// 	SPDLOG_LOGGER_TRACE(logger, "Closing {}", m_socketFd);
// 	::close(m_socketFd);
// }

}
