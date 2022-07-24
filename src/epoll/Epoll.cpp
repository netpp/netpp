//
// Created by gaojian on 2020/7/5.
//

#include "epoll/Epoll.h"
#include "support/Log.h"
#include "internal/stub/IO.h"
#include "internal/stub/Epoll.h"
#include "error/Exception.h"
#include "epoll/EpollEventHandler.h"
#include <climits>
extern "C" {
#include <sys/epoll.h>
}

namespace netpp {
Epoll::Epoll()
	: m_activeEvents(32)
{
	m_epollFd = ::epoll_create1(EPOLL_CLOEXEC);
	if (m_epollFd == -1)
	{
		switch (errno)
		{
			case EINVAL:
				break;
			case EMFILE:
			case ENFILE:
				throw InternalException(Error::FileDescriptorLimited);
			case ENOMEM:
				throw InternalException(Error::MemoryUnavailable);
			default:
				break;
		}
	}
}

Epoll::~Epoll()
{
	::close(m_epollFd);
}

void Epoll::poll()
{
	restartWait:
	int currentVectorSize = static_cast<int>(m_activeEvents.size());
	int nums = ::epoll_wait(m_epollFd, &m_activeEvents[0], currentVectorSize, -1);

	if (nums == -1)
	{
		LOG_INFO("epoll_wait failed, poll next time");
		switch (errno)
		{
			case EINTR:
				goto restartWait;
			case EBADF:
			case EFAULT:
			case EINVAL:
			default:
				break;
		}
		return;
	}
	for (unsigned i = 0; i < static_cast<unsigned>(nums); ++i)
	{
		uint32_t event = m_activeEvents[i].events;
		auto evHandler = static_cast<EpollEventHandler *>(m_activeEvents[i].data.ptr);
		evHandler->handleEvents(event);
	}
}

void Epoll::addEvent(EpollEventHandler *eventAdapter)
{
	LOG_TRACE("add channel id {} with event {}", channelFd, static_cast<uint32_t>(event.events));
	int channelFd = eventAdapter->fileDescriptor();
	::epoll_event *event = eventAdapter->interestedEvent();
	realCtl(m_epollFd, EPOLL_CTL_ADD, channelFd, event);
}

void Epoll::updateEvent(EpollEventHandler *eventAdapter)
{
	int channelFd = eventAdapter->fileDescriptor();
	::epoll_event *event = eventAdapter->interestedEvent();
	LOG_TRACE("update channel id {} with event {}", channelFd, static_cast<uint32_t>(event.events));
	realCtl(m_epollFd, EPOLL_CTL_MOD, channelFd, event);
}

void Epoll::removeEvent(EpollEventHandler *eventAdapter)
{
	int channelFd = eventAdapter->fileDescriptor();
	if (realCtl(m_epollFd, EPOLL_CTL_DEL, channelFd, nullptr) != 0)
		LOG_ERROR("Failed to remove channel: {} id:{}", std::strerror(errno), channelFd);
}

int Epoll::realCtl(int epfd, int op, int fd, struct ::epoll_event *event)
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
			case ENOMEM:
			case ENOSPC:
			case EPERM:
			default:
				break;
		}
	}
	return ret;
}
}
