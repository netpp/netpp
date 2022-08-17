//
// Created by 17271 on 2022/6/8.
//

#include "epoll/EpollEventHandler.h"
#include "eventloop/EventLoop.h"
#include "epoll/Epoll.h"
extern "C" {
#include <sys/epoll.h>
}

namespace netpp {
EpollEventHandler::EpollEventHandler(EventLoop *loop)
		: _loopThisHandlerLiveIn{loop}, m_registeredToEpoll{false}, m_interestedEvents{std::make_unique<::epoll_event>()}
{
	static_assert(static_cast<int>(EpollEv::IN) == EPOLLIN);
	static_assert(static_cast<int>(EpollEv::OUT) == EPOLLOUT);
	static_assert(static_cast<int>(EpollEv::RDHUP) == EPOLLRDHUP);
	static_assert(static_cast<int>(EpollEv::PRI) == EPOLLPRI);
	static_assert(static_cast<int>(EpollEv::ERR) == EPOLLERR);
	static_assert(static_cast<int>(EpollEv::HUP) == EPOLLHUP);

	m_interestedEvents->events = 0;
	m_interestedEvents->data.ptr = this;

	_poll = loop->getPoll();
}

EpollEventHandler::~EpollEventHandler() = default;

void EpollEventHandler::handleEvents(uint32_t events)
{
	if (events & EPOLLERR)
		handleErr();
	if (events & EPOLLRDHUP)
		handleRdhup();
	if (events & EPOLLIN)
		handleIn();
	if (events & EPOLLOUT)
		handleOut();
	if (events & EPOLLPRI)
		handlePri();
	if (events & EPOLLHUP)
		handleHup();
}

void EpollEventHandler::activeEvents(uint32_t event)
{
	m_interestedEvents->events |= event;
	if (m_registeredToEpoll)
		_poll->updateEvent(this);
	else
	{
		_poll->addEvent(this);
		m_registeredToEpoll = true;
	}
}

void EpollEventHandler::deactivateEvents(uint32_t event)
{
	m_interestedEvents->events &= ~event;
	_poll->updateEvent(this);
}

void EpollEventHandler::setEvents(uint32_t event)
{
	m_interestedEvents->events = event;
	if (m_registeredToEpoll)
		_poll->updateEvent(this);
	else
	{
		_poll->addEvent(this);
		m_registeredToEpoll = true;
	}
}

void EpollEventHandler::disableEvent()
{
	_poll->removeEvent(this);
	m_registeredToEpoll = false;
}
}
