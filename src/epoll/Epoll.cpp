//
// Created by gaojian on 2020/7/5.
//

#include <sys/epoll.h>
#include <cstring>
#include "epoll/Epoll.h"
#include "Log.h"
#include "epoll/EpollEvent.h"

namespace netpp::epoll {
Epoll::Epoll()
	: m_activeEvents(4)
{
	if ((m_epfd = ::epoll_create(EPOLL_CLOEXEC)) == -1)
		SPDLOG_LOGGER_CRITICAL(logger, "Failed to create epoll {}", strerror(errno));
}

Epoll::~Epoll()
{
	::close(m_epfd);
}

std::vector<EpollEvent *> Epoll::poll()
{
	int nums = ::epoll_wait(m_epfd, &m_activeEvents[0], m_activeEvents.size(), -1);

	if (nums == m_activeEvents.size())
	{
		SPDLOG_LOGGER_INFO(logger, "active events array resize to {}", m_activeEvents.size() * 2);
		m_activeEvents.resize(m_activeEvents.size() * 2);
	}

	std::vector<EpollEvent *> activeChannels;
	for (int i = 0; i < nums; ++i)
	{
		uint32_t event = m_activeEvents[i].events;
		auto epollEvent = static_cast<EpollEvent *>(m_activeEvents[i].data.ptr);
		epollEvent->updateActiveEvents(event);
		activeChannels.emplace_back(epollEvent);
	}
	return activeChannels;
}

void Epoll::updateEvent(EpollEvent *channelEvent)
{
	int channelFd = channelEvent->fd();
	epoll_event event = channelEvent->epollEvent();
	if (_events.find(channelFd) == _events.end())		// add channel
	{
		SPDLOG_LOGGER_TRACE(logger, "add channel id:{} events:{}", channelFd);
		_events[channelFd] = channelEvent;
		if (epoll_ctl(m_epfd, EPOLL_CTL_ADD, channelFd, &event) != 0/* && errno == EEXIST*/)
			SPDLOG_LOGGER_ERROR(logger, "Failed to add channel: {} id:{} event{}", strerror(errno), channelFd);
	}
	else	// update channel
	{
		if (_events[channelFd] != channelEvent)
		{
			SPDLOG_LOGGER_TRACE(logger, "Channel for socket {} is changed", channelFd);
			_events[channelFd] = channelEvent;
		}
		SPDLOG_LOGGER_TRACE(logger, "update channel id:{} events:{}", channelFd);
		if (epoll_ctl(m_epfd, EPOLL_CTL_MOD, channelFd, &event) != 0/* && errno == ENOENT*/)
			SPDLOG_LOGGER_ERROR(logger, "Failed to update channel: {} id:{} event{}", strerror(errno), channelFd);
	}
}

void Epoll::removeEvent(EpollEvent *channelEvent)
{
	int channelFd = channelEvent->fd();
	if (_events.find(channelFd) != _events.end())		// add channel
	{
		SPDLOG_LOGGER_TRACE(logger, "remove channel id:{}", channelFd);
		_events.erase(channelFd);
		if (epoll_ctl(m_epfd, EPOLL_CTL_DEL, channelFd, nullptr) != 0)
			SPDLOG_LOGGER_ERROR(logger, "Failed to remove channel: {} id:{}", strerror(errno), channelFd);
	}
	else
	{
		SPDLOG_LOGGER_WARN(logger, "No such a channel: id:{}", channelFd);
	}
}
}
