//
// Created by gaojian on 2020/7/5.
//

#include "epoll/EpollEvent.h"
#include "epoll/EventHandler.h"
#include "epoll/Epoll.h"
#include "support/Log.h"

namespace netpp::internal::epoll {
EpollEvent::EpollEvent(Epoll *poll, std::weak_ptr<EventHandler> handler, int fd)
	: _poll{poll}, _eventHandler{handler}, _watchingFd{fd}, activeEvents{0}
{
	// TODO: default watching event should be set by EventHandlers
	m_watchingEvents.events = EPOLLRDHUP | EPOLLERR;
	m_watchingEvents.data.ptr = this;
}

void EpollEvent::handleEvents()
{
	auto handler = _eventHandler.lock();
	// TODO: should handle more events
	if (handler)
	{
		if (activeEvents & EPOLLERR)
			handler->handleError();
		if (activeEvents & EPOLLRDHUP)
			handler->handleClose();
		if (activeEvents & EPOLLIN)
			handler->handleRead();
		if (activeEvents & EPOLLOUT)
			handler->handleWrite();
	}
}

void EpollEvent::setEnableWrite(bool enable)
{
	LOG_TRACE("Set {} enable write {}", _watchingFd, enable);
	if (enable)
		m_watchingEvents.events |= EPOLLOUT;
	else
		m_watchingEvents.events &= ~EPOLLOUT;
	_poll->updateEvent(this);
}

void EpollEvent::setEnableRead(bool enable)
{
	LOG_TRACE("Set {} enable read {}", _watchingFd, enable);
	if (enable)
		m_watchingEvents.events |= EPOLLIN;
	else
		m_watchingEvents.events &= ~EPOLLIN;
	_poll->updateEvent(this);
}

void EpollEvent::deactiveEvents()
{
	_poll->removeEvent(this);
}
}
