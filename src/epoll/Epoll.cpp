//
// Created by gaojian on 2020/7/5.
//

#include <cstring>
#include "epoll/Epoll.h"
#include "support/Log.h"
#include "epoll/EpollEvent.h"
#include "stub/IO.h"
extern "C" {
#include <sys/epoll.h>
}

namespace netpp::epoll {
Epoll::Epoll()
	: m_activeEvents(4)
{
	if ((m_epfd = ::epoll_create1(EPOLL_CLOEXEC)) == -1)
		LOG_CRITICAL("Failed to create epoll {}", std::strerror(errno));
}

Epoll::~Epoll()
{
	stub::close(m_epfd);
}

std::vector<EpollEvent *> Epoll::poll()
{
	int nums = ::epoll_wait(m_epfd, &m_activeEvents[0], m_activeEvents.size(), 500);

	if (nums == m_activeEvents.size())
	{
		LOG_INFO("active events array resize to {}", m_activeEvents.size() * 2);
		m_activeEvents.resize(m_activeEvents.size() * 2);
	}

	std::vector<EpollEvent *> activeChannels;
	for (int i = 0; i < nums; ++i)
	{
		uint32_t event = m_activeEvents[i].events;
		auto epollEvent = static_cast<EpollEvent *>(m_activeEvents[i].data.ptr);
		epollEvent->setActiveEvents(event);
		activeChannels.emplace_back(epollEvent);
	}
	return activeChannels;
}

void Epoll::updateEvent(EpollEvent *channelEvent)
{
	int channelFd = channelEvent->fd();
	::epoll_event event = channelEvent->watchingEvent();
	int op;
	auto evIt = _events.find(channelFd);
	if (evIt == _events.end())		// add channel
	{
		LOG_TRACE("add channel id {} with event {}", channelFd, static_cast<uint32_t>(event.events));
		_events.insert({channelFd, channelEvent});
		op = EPOLL_CTL_ADD;
	}
	else	// update channel
	{
		if (evIt->second != channelEvent)
		{
			LOG_WARN("Channel for fd {} is changed", channelFd);
			_events.insert_or_assign(channelFd, channelEvent);
		}
		LOG_TRACE("update channel id {} with event {}", channelFd, static_cast<uint32_t>(event.events));
		op = EPOLL_CTL_MOD;
	}
	if (::epoll_ctl(m_epfd, op, channelFd, &event) == -1)
		LOG_ERROR("Failed to update channel {}", std::strerror(errno));
}

void Epoll::removeEvent(EpollEvent *channelEvent)
{
	int channelFd = channelEvent->fd();
	auto evIt = _events.find(channelFd);
	if (evIt != _events.end())		// add channel
	{
		LOG_TRACE("remove channel id:{}", channelFd);
		_events.erase(evIt);
		if (::epoll_ctl(m_epfd, EPOLL_CTL_DEL, channelFd, nullptr) != 0)
			LOG_ERROR("Failed to remove channel: {} id:{}", std::strerror(errno), channelFd);
	}
	else
	{
		LOG_INFO("No such a channel {}, failed to remove", channelFd);
	}
}
}
