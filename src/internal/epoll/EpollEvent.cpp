//
// Created by gaojian on 2020/7/5.
//

#include "internal/epoll/EpollEvent.h"
#include <utility>
#include "internal/epoll/EventHandler.h"
#include "internal/epoll/Epoll.h"

namespace netpp::internal::epoll {
EpollEvent::EpollEvent(Epoll *poll, std::weak_ptr<EventHandler> handler, int fd)
	: _poll{poll}, _eventHandler{std::move(handler)},
	  _watchingFd{fd}, m_watchingEvents{0, {this}}, m_activeEvents{0}
{
	_poll->addEvent(this);
}

EpollEvent::~EpollEvent()
{
	_poll->removeEvent(this);
}

void EpollEvent::handleEvents()
{
	auto handler = _eventHandler.lock();
	if (handler)
	{
		if (m_activeEvents & EPOLLERR)
			handler->handleErr();
		if (m_activeEvents & EPOLLRDHUP)
			handler->handleRdhup();
		if (m_activeEvents & EPOLLIN)
			handler->handleIn();
		if (m_activeEvents & EPOLLOUT)
			handler->handleOut();
		if (m_activeEvents & EPOLLPRI)
			handler->handlePri();
		if (m_activeEvents & EPOLLHUP)
			handler->handleHup();
	}
}

void EpollEvent::activeEvents(uint32_t event)
{
	m_watchingEvents.events |= event;
	_poll->updateEvent(this);
}

void EpollEvent::deactivateEvents(uint32_t event)
{
	m_watchingEvents.events &= ~event;
	_poll->updateEvent(this);
}

void EpollEvent::setEvents(uint32_t event)
{
	m_watchingEvents.events = event;
	_poll->updateEvent(this);
}
}
