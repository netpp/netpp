//
// Created by 17271 on 2022/6/5.
//

#include "iodevice/UdpSocket.h"
#include "support/Log.h"
#include "support/Util.h"
#include "error/Exception.h"
#include "buffer/BufferIOConversion.h"
#include "buffer/TransferBuffer.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
}

namespace netpp {
UdpSocket::UdpSocket()
		: m_udpFd{-1}
{}

UdpSocket::UdpSocket(int sockFd, const Address &address)
		: m_peerAddress(address), m_udpFd{sockFd}
{}

UdpSocket::~UdpSocket() noexcept
{
	realClose();
}

Address UdpSocket::receive(std::unique_ptr<TransferBuffer> &&buffer)
{
	auto bufferConverter = buffer->receiveBufferForIO();
	::msghdr *msg = bufferConverter->msghdr();
	::sockaddr_in receivedFrom;
	msg->msg_name = &receivedFrom;
	msg->msg_namelen = sizeof(receivedFrom);

	std::size_t num = realRecv(msg);
	bufferConverter->adjustByteArray(static_cast<std::size_t>(num));
	return toAddress(receivedFrom);
}

std::size_t UdpSocket::send(std::unique_ptr<TransferBuffer> &&buffer, const Address &address)
{
	auto bufferConverter = buffer->sendBufferForIO();
	::msghdr *msg = bufferConverter->msghdr();
	auto destination = toSockAddress(address);
	msg->msg_name = &destination;
	msg->msg_namelen = sizeof(destination);
	std::size_t actualSend = realSend(msg);

	auto sendSize = static_cast<std::size_t>(actualSend);
	bufferConverter->adjustByteArray(sendSize);
	return sendSize;
}

void UdpSocket::open()
{
	m_udpFd = ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
	if (m_udpFd == -1)
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

void UdpSocket::close()
{
	realClose();
}

void UdpSocket::bind(const Address &address)
{
	if (m_udpFd != -1)
	{
		sockaddr_in inetAddress = toSockAddress(address);
		int ret = ::bind(m_udpFd, reinterpret_cast<const sockaddr *>(&inetAddress), sizeof(::sockaddr_in));
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

SocketState UdpSocket::state()
{
	return SocketState::E_Unknown;
}

void UdpSocket::shutdownWrite()
{
	int ret = ::shutdown(m_udpFd, SHUT_WR);
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

bool UdpSocket::connect(const Address &address)
{
	sockaddr_in inetAddress = toSockAddress(address);
	int ret = ::connect(m_udpFd, reinterpret_cast<const ::sockaddr *>(&inetAddress), sizeof(::sockaddr_in));
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
	else
	{
		m_connectFlag.test_and_set();
	}
	return true;
}

std::size_t UdpSocket::realRecv(::msghdr *msg)
{
	if (msg->msg_name == nullptr && !m_connectFlag.test())
		return 0;

	retryReceive:
	ssize_t ret = ::recvmsg(m_udpFd, msg, 0);
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

std::size_t UdpSocket::realSend(::msghdr *msg)
{
	if (msg->msg_name == nullptr && !m_connectFlag.test())
		return 0;

	retrySend:
	ssize_t ret = ::sendmsg(m_udpFd, msg, MSG_NOSIGNAL);
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
	return static_cast<std::size_t>(ret);
}

void UdpSocket::realClose()
{
	if (m_udpFd != -1)
	{
		::close(m_udpFd);
		m_udpFd = -1;
	}
}
}
