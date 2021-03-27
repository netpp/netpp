#include "internal/stub/Socket.h"
#include "internal/support/Log.h"
#include "error/Exception.h"
extern "C" {
#include <sys/socket.h>
#include <cerrno>
}

namespace netpp::internal::stub {
int socket(int domain, int type, int protocol)
{
	int fd = ::socket(domain, type, protocol);
	if (fd == -1)
	{
		LOG_WARN("create socket failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EACCES:
			case EAFNOSUPPORT:
				throw error::SocketException(errno);
			case EINVAL:
			case EPROTONOSUPPORT:
				break;
			case EMFILE:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
				throw error::ResourceLimitException(errno);
		}
	}
	return fd;
}

int bind(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen)
{
	int ret = ::bind(sockfd, addr, addrlen);
	if (ret == -1)
	{
		LOG_WARN("bind address failed due to {}", std::strerror(errno));
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
			case ENOMEM:
				throw error::ResourceLimitException(errno);
		}
	}
	return ret;
}

int listen(int sockfd, int backlog)
{
	int ret = ::listen(sockfd, backlog);
	if (ret == -1)
	{
		LOG_WARN("listen failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EBADF:
			case EDESTADDRREQ:
			case ENOTSOCK:
			case EOPNOTSUPP:
			case EACCES:
				throw error::SocketException(errno);
			case EINVAL:
				break;
			case ENOBUFS:
				throw error::ResourceLimitException(errno);
		}
	}
	return ret;
}

int accept4(int sockfd, struct ::sockaddr *addr, ::socklen_t *addrlen, int flags)
{
	int newSocket = ::accept4(sockfd, addr, addrlen, flags);
	if (newSocket == -1)
	{
		LOG_WARN("accept failed due to {}", std::strerror(errno));
		switch(errno)
		{
			case EAGAIN:
			case EBADF:
			case EFAULT:
			case EINTR:// will automatically restart
			case EINVAL:
			case ENOTSOCK:
			case EOPNOTSUPP:
			case EPROTO:
				break;
			case ECONNABORTED:
				throw error::SocketException(errno);
			case EMFILE:
			case ENFILE:
			case ENOBUFS:
			case ENOMEM:
			case EPERM:
				throw error::ResourceLimitException(errno);
		}
	}
	return newSocket;
}

int connect(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen)
{
	int ret = ::connect(sockfd, addr, addrlen);
	if (ret == -1)
	{
		LOG_WARN("connect failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EACCES:
			case EPERM:
			case EAFNOSUPPORT:
			case EAGAIN:
			case EALREADY:
			case EBADF:
			case EFAULT:
			case EINTR:// can not restart connect immediately, reconnect in loop
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
		}
	}
	return ret;
}

int shutdown(int sockfd, int how) noexcept
{
	int ret = ::shutdown(sockfd, how);
	if (ret == -1)
	{
		LOG_WARN("shutdown {} failed due to {}", sockfd, std::strerror(errno));
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
		LOG_WARN("get socket {} options failed due to {}", sockfd, std::strerror(errno));
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
