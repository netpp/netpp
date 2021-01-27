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
	EpollEvent(Epoll *poll, std::weak_ptr<EventHandler> handler, int fd);
	~EpollEvent();

	int fd() const { return _watchingFd; }
	
	/**
	 * @brief Return current activing epoll events
	 */
	inline ::epoll_event watchingEvent() const { return m_watchingEvents; }

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
	int _watchingFd;
	::epoll_event m_watchingEvents;
	uint32_t activeEvents;
};
}

#endif //NETPP_EPOLLEVENT_H
