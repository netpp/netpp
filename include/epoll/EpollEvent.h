//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EPOLLEVENT_H
#define NETPP_EPOLLEVENT_H

#include <memory>

extern "C" {
#include <sys/epoll.h>
}

namespace netpp::epoll {

class Epoll;
class EventHandler;

class EpollEvent {
public:
	enum EventType : unsigned {
		WriteEvent	= EPOLLOUT,
		ReadEvent	= EPOLLIN,
		DisconnEvent= EPOLLRDHUP,
		ErrEvent	= EPOLLERR,
		OutOfBand	= EPOLLPRI
	};

	EpollEvent(Epoll *poll, std::weak_ptr<EventHandler> handler);
	~EpollEvent();

	int fd() const;
	
	/**
	 * @brief Return current activing epoll events
	 */
	inline ::epoll_event activeEvent() const { return m_events; }

	/**
	 * @brief Set active events, used by poller
	 * 
	 * @param events activing events
	 */
	void setActiveEvents(uint32_t events) { activeEvents = events; }

	void handleEvents();

	/**
	 * @brief Must call this to remove event from epoll before desctruction
	 */
	void disableEvents();

	/**
	 * @brief let epoll watch readable event
	 */

	void setEnableRead(bool enable);
	/**
	 * @brief let epoll watch writeable event
	 */
	void setEnableWrite(bool enable);
private:
	Epoll *_poll;
	std::weak_ptr<EventHandler> _eventHandler;
	::epoll_event m_events;
	uint32_t activeEvents;
};
}

#endif //NETPP_EPOLLEVENT_H
