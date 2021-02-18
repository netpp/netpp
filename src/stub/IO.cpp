#include "stub/IO.h"
#include "support/Log.h"
#include "error/Exception.h"
extern "C" {
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
}

namespace netpp::internal::stub {
int close(int fd) noexcept
{
	int ret = ::close(fd);
	if (ret == -1)
	{
		LOG_WARN("close {} failed due to {}", fd, std::strerror(errno));
		// No need handle close failed, just log it
		switch (errno)
		{
			case EBADF:
			case EINTR:	// man page: close() must not be called again after an EINTR error
			case EIO:
			case ENOSPC:
			case EDQUOT:
				break;
		}
	}
	return ret;
}

::ssize_t write(int fd, const void *buf, ::size_t count) noexcept
{
	::ssize_t size = ::write(fd, buf, count);
	if (size == -1)
	{
		LOG_WARN("write to {} failed due to {}", fd, std::strerror(errno));
		// No need handle write failed, just log it
		switch (errno)
		{
			case EAGAIN:
			case EBADF:
			case EDESTADDRREQ:
			case EDQUOT:
			case EFAULT:
			case EFBIG:
			case EINTR:// will automatically restart
			case EINVAL:
			case EIO:
			case ENOSPC:
			case EPERM:
			case EPIPE:
				break;
		}
	}
	return size;
}

::ssize_t read(int fd, void *buf, ::size_t count) noexcept
{
	::ssize_t size = ::read(fd, buf, count);
	if (size == -1)
	{
		LOG_WARN("write to {} failed due to {}", fd, std::strerror(errno));
		switch (errno)
		{
			case EAGAIN:
			case EBADF:
			case EFAULT:
			case EINTR:// will automatically restart
			case EINVAL:
			case EIO:
			case EISDIR:
				break;
		}
	}
	return size;
}

int pipe2(int pipefd[2], int flags) noexcept
{
	int ret = ::pipe2(pipefd, O_NONBLOCK);
	if (ret == -1)
	{
		LOG_WARN("open pipe failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EFAULT:
			case EINVAL:
			case EMFILE:
			case ENFILE:
				break;
		}
	}
	return ret;
}

::ssize_t sendmsg(int sockfd, const struct ::msghdr *msg, int flags)
{
retrySendMsg:
	::ssize_t ret = ::sendmsg(sockfd, msg, flags);
	if (ret == -1)
	{
		LOG_WARN("sendmsg on {} failed due to {}", sockfd, std::strerror(errno));
		switch (errno)
		{
			case EACCES:
			case EAGAIN:
			case EALREADY:
			case EBADF:
			case EFAULT:
			case ENOTSOCK:
			case EDESTADDRREQ:
			case EMSGSIZE:
			case EISCONN:
			case EOPNOTSUPP:
				break;
			case ECONNRESET:
			case ENOTCONN:
			case EPIPE:
				throw error::SocketException(errno);
				break;
			case EINTR:
				LOG_INFO("restart sendmsg");
				goto retrySendMsg;
				break;
			case EINVAL:
			case ENOBUFS:
				break;
			case ENOMEM:
				throw error::ResourceLimitException(errno);
				break;
		}
	}
	return ret;
}

::ssize_t recvmsg(int sockfd, struct ::msghdr *msg, int flags)
{
retryRecvMsg:
	::ssize_t ret = ::recvmsg(sockfd, msg, flags);
	if (ret == -1)
	{
		switch (errno)
		{
			case EAGAIN:
			case EBADF:
			case EFAULT:
			case EINVAL:
			case ENOTSOCK:
				break;
			case ENOMEM:
				throw error::ResourceLimitException(errno);
				break;
			case EINTR:
				LOG_INFO("restart recvmsg");
				goto retryRecvMsg;
				break;
			case ECONNREFUSED:
			case ENOTCONN:
				throw error::SocketException(errno);
				break;
		}
	}
	return ret;
}
}
