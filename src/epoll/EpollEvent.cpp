//
// Created by gaojian on 2020/7/5.
//

#include "epoll/EpollEvent.h"
#include "epoll/EventHandler.h"
#include "epoll/Epoll.h"
#include "Log.h"

namespace netpp::epoll {
EpollEvent::EpollEvent(Epoll *poll, std::weak_ptr<EventHandler> handler)
		: m_events{DisconnEvent | ErrEvent}, activeEvents{0}, _eventHandler{handler}, _poll{poll}
{
	m_events.data.ptr = this;
}

EpollEvent::~EpollEvent()
{}

int EpollEvent::fd() const
{
	auto handler = _eventHandler.lock();
	if (handler)
		return handler->fd();
	return 0;
}

void EpollEvent::handleEvents()
{
	auto handler = _eventHandler.lock();
	if (handler)
	{
		if (activeEvents & ReadEvent)
			handler->handleRead();
		if (activeEvents & WriteEvent)
			handler->handleWrite();
		if (activeEvents & DisconnEvent)
			handler->handleClose();
		if (activeEvents & ErrEvent)
			handler->handleError();
	}
}

void EpollEvent::setEnableWrite(bool enable)
{
	SPDLOG_LOGGER_TRACE(logger, "Set enable write {}", enable);
	if (enable)
		m_events.events |= WriteEvent;
	else
		m_events.events &= ~WriteEvent;
	_poll->updateEvent(this);
}

void EpollEvent::setEnableRead(bool enable)
{
	SPDLOG_LOGGER_TRACE(logger, "Set enable read {}", enable);
	if (enable)
		m_events.events |= ReadEvent;
	else
		m_events.events &= ~ReadEvent;
	_poll->updateEvent(this);
}

void EpollEvent::disableEvents()
{
	_poll->removeEvent(this);
}
}