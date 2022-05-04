//
// Created by gaojian on 2020/7/5.
//

#include "internal/epoll/Epoll.h"
#include "internal/support/Log.h"
#include "internal/epoll/EpollEvent.h"
#include "internal/stub/IO.h"
#include "internal/stub/Epoll.h"
#include "error/Exception.h"
#include <climits>

namespace netpp::internal::epoll {
Epoll::Epoll()
	: m_activeEvents(32)
{
	m_epollFd = stub::epoll_create1(EPOLL_CLOEXEC);
}

Epoll::~Epoll()
{
	stub::close(m_epollFd);
}

std::vector<internal::epoll::EpollEvent *>::size_type Epoll::poll(std::vector<internal::epoll::EpollEvent *> &channels)
{
	int currentVectorSize = static_cast<int>(m_activeEvents.size());
	int nums = stub::epoll_wait(m_epollFd, &m_activeEvents[0], currentVectorSize, -1);

	if (nums == -1)
	{
		LOG_INFO("epoll_wait failed, poll next time");
		return 0;
	}
	// if event vector is full, expand it, limit vector's max size is less than INT_MAX
	if ((nums == currentVectorSize) && ((currentVectorSize * 2) < INT_MAX))
	{
		LOG_INFO("activeEvents events array resize to {}", m_activeEvents.size() * 2);
		m_activeEvents.resize(m_activeEvents.size() * 2);
		channels.resize(m_activeEvents.size() * 2);
	}
	using VecSize = std::vector<internal::epoll::EpollEvent *>::size_type;
	auto active = static_cast<VecSize>(nums);
	for (VecSize i = 0; i < active; ++i)
	{
		uint32_t event = m_activeEvents[i].events;
		auto epollEvent = static_cast<EpollEvent *>(m_activeEvents[i].data.ptr);
		epollEvent->setActiveEvents(event);
		channels[i] = epollEvent;
	}
	return active;
}

void Epoll::addEvent(EpollEvent *eventAdapter) const
{
	try
	{
		LOG_TRACE("add channel id {} with event {}", channelFd, static_cast<uint32_t>(event.events));
		int channelFd = eventAdapter->fd();
		::epoll_event event = eventAdapter->watchingEvent();
		stub::epoll_ctl(m_epollFd, EPOLL_CTL_ADD, channelFd, &event);
	}
	catch (error::ResourceLimitException &rle)
	{
		LOG_ERROR("Exception while add events");
	}
}

void Epoll::updateEvent(EpollEvent *eventAdapter) const
{
	try
	{
		int channelFd = eventAdapter->fd();
		::epoll_event event = eventAdapter->watchingEvent();
		LOG_TRACE("update channel id {} with event {}", channelFd, static_cast<uint32_t>(event.events));
		stub::epoll_ctl(m_epollFd, EPOLL_CTL_MOD, channelFd, &event);
	}
	catch (error::ResourceLimitException &rle)
	{
		LOG_ERROR("Exception while update events");
	}
}

void Epoll::removeEvent(EpollEvent *eventAdapter) const
{
	try
	{
		int channelFd = eventAdapter->fd();
		if (stub::epoll_ctl(m_epollFd, EPOLL_CTL_DEL, channelFd, nullptr) != 0)
			LOG_ERROR("Failed to remove channel: {} id:{}", std::strerror(errno), channelFd);
	}
	catch (error::ResourceLimitException &rle)
	{
		LOG_ERROR("Exception while remove events");
	}
}
}
