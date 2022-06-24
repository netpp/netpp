//
// Created by 17271 on 2022/6/5.
//

#include <cstring>
#include "iodevice/TcpSocket.h"
#include "support/Log.h"
#include "support/Util.h"
#include "error/Exception.h"

extern "C" {
#include <sys/socket.h>
#include <netinet/in.h>
}

namespace netpp {
TcpSocket::TcpSocket()
		: m_tcpFd{-1}
{}

TcpSocket::TcpSocket(int sockFd, const Address &address)
		: m_peerAddress(address), m_tcpFd{sockFd}
{}

TcpSocket::~TcpSocket() noexcept
{
	realClose();
}

void TcpSocket::open()
{
	m_tcpFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
	if (m_tcpFd == -1)
	{
		LOG_WARN("create socket failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EACCES: throw InternalException(Error::PermissionDenied);
			case EAFNOSUPPORT:
			case EPROTONOSUPPORT: throw InternalException(Error::NotSupport);
			case EINVAL: break;
			case EMFILE:
			case ENFILE: throw InternalException(Error::FileDescriptorLimited);
			case ENOBUFS:
			case ENOMEM: throw InternalException(Error::MemoryUnavailable);
		}
	}
	LOG_DEBUG("Socket fd: {}", m_tcpFd);
}

void TcpSocket::close()
{
	realClose();
}

void TcpSocket::bind(const Address &address)
{
	if (m_tcpFd != -1)
	{
		sockaddr_in inetAddress = toSockAddress(address);
		int ret = ::bind(m_tcpFd, reinterpret_cast<const sockaddr *>(&inetAddress), sizeof(::sockaddr_in));
		if (ret == -1)
		{
			LOG_WARN("bind address failed due to {}", std::strerror(errno));
			switch (errno)
			{
				case EACCES: throw InternalException(Error::PermissionDenied);
				case ENOTSOCK:
				case EBADF: throw InternalException(Error::BadFD);
				case EINVAL: break;
				case EADDRNOTAVAIL:
				case EFAULT:
				case ELOOP:
				case ENAMETOOLONG: throw InternalException(Error::InvalidAddress);
				case ENOENT:
				case ENOTDIR:
				case EROFS: throw InternalException(Error::Unknown);
				case EADDRINUSE: throw InternalException(Error::AddressInUse);
				case ENOMEM: throw InternalException(Error::MemoryUnavailable);
			}
		}
	}
}

void TcpSocket::listen()
{
	LOG_TRACE("Start listen");
	int ret = ::listen(m_tcpFd, SOMAXCONN);
	if (ret == -1)
	{
		LOG_WARN("listen failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EADDRINUSE: throw InternalException(Error::AddressInUse);
			case EBADF:
			case ENOTSOCK:
			case EOPNOTSUPP: throw InternalException(Error::BadFD);
		}
	}
}

SocketDevice *TcpSocket::accept()
{
	::sockaddr_in address;
	socklen_t addressSize = sizeof(::sockaddr_in);
	std::memset(&address, 0, addressSize);
	int newSocket = ::accept4(m_tcpFd, reinterpret_cast<::sockaddr *>(&address), &addressSize,
							  SOCK_NONBLOCK | SOCK_CLOEXEC
	);
	if (newSocket == -1)
	{
		LOG_WARN("accept failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EAGAIN:
			case EINVAL:
			case ECONNABORTED: break;
			case EBADF:
			case ENOTSOCK:
			case EOPNOTSUPP:
			case EPROTO: throw InternalException(Error::BadFD);
			case EFAULT: throw InternalException(Error::InvalidAddress);
			case EINTR:// will automatically restart
				break;
		}
		return nullptr;
	}
	else
	{
		LOG_TRACE("Accepted new client with fd {}", newSocket);
		return new TcpSocket(newSocket, toAddress(address));
	}
}

SocketState TcpSocket::state()
{
	int optval;
	auto optlen = static_cast<socklen_t>(sizeof optval);
	if (::getsockopt(m_tcpFd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == 0)
	{
		switch (optval)
		{
			case ETIMEDOUT:
				return SocketState::E_TimeOut;
			case ECONNREFUSED:
				return SocketState::E_ConnectRefused;
			case EINPROGRESS:
				return SocketState::E_InProgress;
			default:
				return SocketState::E_Unknown;
		}
	}
	return SocketState::E_Unknown;
}

void TcpSocket::shutdownWrite()
{
	LOG_TRACE("Shut down write for socket {}", m_socketFd);
	int ret = ::shutdown(m_tcpFd, SHUT_WR);
	if (ret != -1)
	{
		switch (errno)
		{
			case EINVAL: break;
			case EBADF:
			case ENOTCONN:
			case ENOTSOCK: throw InternalException(Error::BadFD);
		}
	}
}

bool TcpSocket::connect(const Address &address)
{
	sockaddr_in inetAddress = toSockAddress(address);
	int ret = ::connect(m_tcpFd, reinterpret_cast<const ::sockaddr *>(&inetAddress), sizeof(::sockaddr_in));
	if (ret == -1)
	{
		LOG_WARN("connect failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EACCES:
			case EPERM: throw InternalException(Error::PermissionDenied);
			case EAFNOSUPPORT:
			case EFAULT:
			case EPROTOTYPE:
			case EADDRINUSE: throw InternalException(Error::InvalidAddress);
			case EAGAIN:
			case EALREADY:
			case EISCONN: break;
			case EBADF:
			case ENOTSOCK: throw InternalException(Error::BadFD);
			case EINTR:// can not restart connect immediately, reconnect in loop
				break;
			case ENETUNREACH: throw InternalException(Error::Unreachable);
			case EADDRNOTAVAIL: throw InternalException(Error::FileDescriptorLimited);
			case ECONNREFUSED: throw InternalException(Error::ConnectRefused);
			case ETIMEDOUT: throw InternalException(Error::Timeout);
			case EINPROGRESS: return false;
		}
	}
	return true;
}

std::size_t TcpSocket::realRecv(::msghdr *msg)
{
	retryReceive:
	ssize_t ret = ::recvmsg(m_tcpFd, msg, 0);
	if (ret == -1)
	{
		switch (errno)
		{
			case EAGAIN:
			case EINVAL: break;
			case EBADF:
			case ENOTCONN:
			case ENOTSOCK: throw InternalException(Error::BadFD);
			case ECONNREFUSED: throw InternalException(Error::ConnectRefused);
			case EFAULT: throw InternalException(Error::InvalidAddress);
			case EINTR: goto retryReceive;
			case ENOMEM: throw InternalException(Error::MemoryUnavailable);
		}
	}
	return static_cast<std::size_t>(ret);
}

std::size_t TcpSocket::realSend(::msghdr *msg)
{
	retrySend:
	ssize_t ret = ::sendmsg(m_tcpFd, msg, MSG_NOSIGNAL);
	if (ret == -1)
	{
		switch (errno)
		{
			case EACCES:
			case EDESTADDRREQ:
			case EFAULT: throw InternalException(Error::InvalidAddress);
			case EAGAIN:
			case EALREADY: break;
			case EBADF:
			case ENOTSOCK:
			case EISCONN:
			case ENOTCONN:
			case EMSGSIZE:
			case EOPNOTSUPP: throw InternalException(Error::BadFD);
			case ECONNRESET:
			case EPIPE: throw InternalException(Error::BrokenConnection);
			case EINTR: goto retrySend;
			case EINVAL: break;
			case ENOBUFS:
			case ENOMEM: throw InternalException(Error::MemoryUnavailable);
		}
	}
}

void TcpSocket::realClose()
{
	if (m_tcpFd != -1)
	{
		::close(m_tcpFd);
		m_tcpFd = -1;
	}
}
}
