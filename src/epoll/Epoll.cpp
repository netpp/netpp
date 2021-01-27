//
// Created by gaojian on 2020/7/5.
//

#include <sys/epoll.h>
#include <cstring>
#include "epoll/Epoll.h"
#include "Log.h"
#include "epoll/EpollEvent.h"
#include "stub/IO.h"

namespace netpp::epoll {
Epoll::Epoll()
	: m_activeEvents(4)
{
	if ((m_epfd = ::epoll_create1(EPOLL_CLOEXEC)) == -1)
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to create epoll {}", std::strerror(errno));
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
		SPDLOG_LOGGER_INFO(logger, "active events array resize to {}", m_activeEvents.size() * 2);
		m_activeEvents.resize(m_activeEvents.size() * 2);
	}

	std::vector<EpollEvent *> activeChannels;
	for (int i = 0; i < nums; ++i)
	{
		uint32_t event = m_activeEvents[i].events;
		SPDLOG_LOGGER_TRACE(logger, "handle event {}", event);
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
	auto evIt = _events.find(channelFd);
	if (evIt == _events.end())		// add channel
	{
		SPDLOG_LOGGER_TRACE(logger, "add channel id {} with event {}", channelFd, static_cast<uint32_t>(event.events));
		_events.insert({channelFd, channelEvent});
		if (::epoll_ctl(m_epfd, EPOLL_CTL_ADD, channelFd, &event) != 0/* && errno == EEXIST*/)
			SPDLOG_LOGGER_ERROR(logger, "Failed to add channel {}", std::strerror(errno));
	}
	else	// update channel
	{
		if (evIt->second != channelEvent)
		{
			SPDLOG_LOGGER_WARN(logger, "Channel for socket {} is changed", channelFd);
			_events.insert_or_assign(channelFd, channelEvent);
		}
		SPDLOG_LOGGER_TRACE(logger, "update channel id {} with event {}", channelFd, static_cast<uint32_t>(event.events));
		if (::epoll_ctl(m_epfd, EPOLL_CTL_MOD, channelFd, &event) != 0/* && errno == ENOENT*/)
			SPDLOG_LOGGER_ERROR(logger, "Failed to update channel: {} id:{} event{}", std::strerror(errno), channelFd);
	}
}

void Epoll::removeEvent(EpollEvent *channelEvent)
{
	int channelFd = channelEvent->fd();
	auto evIt = _events.find(channelFd);
	if (evIt != _events.end())		// add channel
	{
		SPDLOG_LOGGER_TRACE(logger, "remove channel id:{}", channelFd);
		_events.erase(evIt);
		if (::epoll_ctl(m_epfd, EPOLL_CTL_DEL, channelFd, nullptr) != 0)
			SPDLOG_LOGGER_ERROR(logger, "Failed to remove channel: {} id:{}", std::strerror(errno), channelFd);
	}
	else
	{
		SPDLOG_LOGGER_INFO(logger, "No such a channel {}, failed to remove", channelFd);
	}
}
}
