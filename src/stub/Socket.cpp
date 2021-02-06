#include "stub/Socket.h"
#include "Log.h"
#include "error/Exception.h"
#include <cstring>
extern "C" {
#include <sys/socket.h>
#include <errno.h>
}

namespace netpp::stub {
int socket(int domain, int type, int protocol)
{
	int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (fd == -1)
	{
		SPDLOG_LOGGER_WARN(logger, "create socket failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EACCES:
			case EAFNOSUPPORT:
				throw error::SocketException(errno);
				break;
			case EINVAL:
			case EPROTONOSUPPORT:
				break;
			case EMFILE:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
				throw error::ResourceLimitException(errno);
				break;
		}
	}
	return fd;
}

int bind(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen)
{
	int ret = ::bind(sockfd, addr, addrlen);
	if (ret == -1)
	{
		SPDLOG_LOGGER_WARN(logger, "bind address failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EACCES:
			case EBADF:
			case EINVAL:
			case ENOTSOCK:
			case EADDRNOTAVAIL:
			case EFAULT:
			case ELOOP:
			case ENAMETOOLONG:
			case ENOENT:
			case ENOTDIR:
			case EROFS:
				break;
			case EADDRINUSE:
				throw error::SocketException(errno);
				break;
			case ENOMEM:
				throw error::ResourceLimitException(errno);
				break;
		}
	}
	return ret;
}

int listen(int sockfd, int backlog)
{
	int ret = ::listen(sockfd, backlog);
	if (ret == -1)
	{
		SPDLOG_LOGGER_WARN(logger, "listen failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EADDRINUSE:
				throw error::SocketException(errno);
				break;
			case EBADF:
			case ENOTSOCK:
			case EOPNOTSUPP:
				break;
		}
	}
	return ret;
}

int accept4(int sockfd, struct ::sockaddr *addr, ::socklen_t *addrlen, int flags)
{
	int newSocket = ::accept4(sockfd, addr, addrlen, flags);
	if (newSocket == -1)
	{
		SPDLOG_LOGGER_WARN(logger, "accept failed due to {}", std::strerror(errno));
		switch(errno)
		{
			case EAGAIN:
			case EBADF:
			case EFAULT:
			case EINTR:
			case EINVAL:
			case ENOTSOCK:
			case EOPNOTSUPP:
			case EPROTO:
				break;
			case ECONNABORTED:
				throw error::SocketException(errno);
				break;
			case EMFILE:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
			case EPERM:
				throw error::ResourceLimitException(errno);
				break;
		}
	}
	return newSocket;
}

int connect(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen)
{
	int ret = ::connect(sockfd, addr, addrlen);
	if (ret == -1)
	{
		SPDLOG_LOGGER_WARN(logger, "connect failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EACCES:
			case EPERM:
			case EAFNOSUPPORT:
			case EAGAIN:
			case EALREADY:
			case EBADF:
			case EFAULT:
			case EINTR:
			case EISCONN:
			case ENOTSOCK:
			case EPROTOTYPE:
				break;
			case EADDRINUSE:
			case EADDRNOTAVAIL:
			case ECONNREFUSED:
			case EINPROGRESS:
			case ENETUNREACH:
			case ETIMEDOUT:
				throw error::SocketException(errno);
				break;
		}
	}
	return ret;
}

int shutdown(int sockfd, int how) noexcept
{
	int ret = ::shutdown(sockfd, how);
	if (ret == -1)
	{
		SPDLOG_LOGGER_WARN(logger, "shutdown {} failed due to {}", sockfd, std::strerror(errno));
		switch (errno)
		{
			case EBADF:
			case EINVAL:
			case ENOTCONN:
			case ENOTSOCK:
				break;
		}
	}
	return ret;
}

int getsockopt(int sockfd, int level, int optname, void *optval, ::socklen_t *optlen) noexcept
{
	int ret = ::getsockopt(sockfd, level, optname, optval, optlen);
	if (ret == -1)
	{
		SPDLOG_LOGGER_WARN(logger, "get socket {} options failed due to {}", sockfd, std::strerror(errno));
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
	return ret;
}
}
