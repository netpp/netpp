#include "internal/stub/Epoll.h"
#include "error/Exception.h"
#include "internal/support/Log.h"
extern "C" {
#include <sys/epoll.h>
#include <cerrno>
}

namespace netpp::internal::stub {
/*int epoll_create(int size)
{
	int ret = ::epoll_create(size);
	if (ret == -1)
	{
		switch (errno)
		{
			case EINVAL:
				break;
			case EMFILE:
			case ENFILE:
			case ENOMEM:
				throw error::ResourceLimitException(errno);
			default:
				break;
		}
	}
	return ret;
}*/

int epoll_create1(int flags)
{
	int ret = ::epoll_create1(flags);
	if (ret == -1)
	{
		LOG_WARN("create epoll failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EINVAL:
				break;
			case EMFILE:
			case ENFILE:
			case ENOMEM:
				throw error::ResourceLimitException(errno);
			default:
				break;
		}
	}
	return ret;
}

int epoll_ctl(int epfd, int op, int fd, struct ::epoll_event *event)
{
	int ret = ::epoll_ctl(epfd, op, fd, event);
	if (ret == -1)
	{
		LOG_WARN("control epoll failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EBADF:
			case EEXIST:
			case EINVAL:
			case ELOOP:
			case ENOENT:
				break;
			case ENOMEM:
			case ENOSPC:
				throw error::ResourceLimitException(errno);
			case EPERM:
			default:
				break;
		}
	}
	return ret;
}

int epoll_wait(int epfd, struct ::epoll_event *events, int maxevents, int timeout) noexcept
{
restartWait:
	int ret = ::epoll_wait(epfd, events, maxevents, timeout);
	if (ret == -1)
	{
		LOG_WARN("wait for epoll event failed due to {}", std::strerror(errno));
		switch (errno)
		{
			case EBADF:
			case EFAULT:
			case EINVAL:
				break;
			case EINTR:
				goto restartWait;
			default:
				break;
		}
	}
	return ret;
}

/*int epoll_pwait(int epfd, struct ::epoll_event *events, int maxevents, int timeout, const ::sigset_t *sigmask)
{
	int ret = ::epoll_wait(epfd, events, maxevents, timeout, sigmask);
	if (ret == -1)
	{
		switch (errno)
		{
			case EBADF:
			case EFAULT:
			case EINTR:
			case EINVAL:
			default:
				break;
		}
	}
	return ret;
}*/
}
