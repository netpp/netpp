//
// Created by gaojian on 2020/7/5.
//

#include "internal/epoll/EpollEvent.h"

#include <utility>
#include "internal/epoll/EventHandler.h"
#include "internal/epoll/Epoll.h"

namespace netpp::internal::epoll {
EpollEvent::EpollEvent(Epoll *poll, std::weak_ptr<EventHandler> handler, int fd)
	: _poll{poll}, _eventHandler{std::move(handler)}, _watchingFd{fd}, activeEvents{0}
{
	m_watchingEvents.events = 0;
	m_watchingEvents.data.ptr = this;
}

void EpollEvent::handleEvents()
{
	auto handler = _eventHandler.lock();
	if (handler)
	{
		if (activeEvents & EPOLLERR)
			handler->handleErr();
		if (activeEvents & EPOLLRDHUP)
			handler->handleRdhup();
		if (activeEvents & EPOLLIN)
			handler->handleIn();
		if (activeEvents & EPOLLOUT)
			handler->handleOut();
		if (activeEvents & EPOLLPRI)
			handler->handlePri();
		if (activeEvents & EPOLLHUP)
			handler->handleHup();
	}
}

void EpollEvent::active(std::initializer_list<EpollEv> events)
{
	for (auto event : events)
	{
		switch (event)
		{
			case EpollEv::IN: m_watchingEvents.events |= EPOLLIN;		break;
			case EpollEv::OUT: m_watchingEvents.events |= EPOLLOUT;	break;
			case EpollEv::RDHUP: m_watchingEvents.events |= EPOLLRDHUP;	break;
			case EpollEv::PRI: m_watchingEvents.events |= EPOLLPRI;	break;
			case EpollEv::ERR: m_watchingEvents.events |= EPOLLERR;	break;
			case EpollEv::HUP: m_watchingEvents.events |= EPOLLHUP;	break;
		}
	}
	_poll->updateEvent(this);
}

void EpollEvent::deactive(std::initializer_list<EpollEv> events)
{
	for (auto event : events)
	{
		switch (event)
		{
			case EpollEv::IN: m_watchingEvents.events &= ~EPOLLIN;	break;
			case EpollEv::OUT: m_watchingEvents.events &= ~EPOLLOUT;	break;
			case EpollEv::RDHUP: m_watchingEvents.events &= ~EPOLLRDHUP;	break;
			case EpollEv::PRI: m_watchingEvents.events &= ~EPOLLPRI;	break;
			case EpollEv::ERR: m_watchingEvents.events &= ~EPOLLERR;	break;
			case EpollEv::HUP: m_watchingEvents.events &= ~EPOLLHUP;	break;
		}
	}
	_poll->updateEvent(this);
}

void EpollEvent::disable()
{
	_poll->removeEvent(this);
}
}
