//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EPOLLEVENT_H
#define NETPP_EPOLLEVENT_H

#include <memory>

extern "C" {
#include <sys/epoll.h>
}

namespace netpp::internal::epoll {
class Epoll;
class EventHandler;

/**
 * @brief Supported epoll events
 * 
 */
enum EpollEv {
	NOEV = 0,
	/** @brief EPOLLIN event */
	IN		= EPOLLIN,
	/** @brief EPOLLOUT event */
	OUT		= EPOLLOUT,
	/** @brief EPOLLRDHUP event */
	RDHUP	= EPOLLRDHUP,
	/**
	 * @brief EPOLLPRI event
	 * @todo out-of-band data are not handled
	 * */
	PRI		= EPOLLPRI,
	/** @brief EPOLLERR event */
	ERR		= EPOLLERR,
	/** @brief EPOLLHUP event */
	HUP		= EPOLLHUP
};

/**
 * @brief The poll event handler
 */
class EpollEvent {
	friend class Epoll;
public:
	EpollEvent(Epoll *poll, std::weak_ptr<EventHandler> handler, int fd);
	~EpollEvent();

	/**
	 * @brief Get watching fd of this event
	 * @return file descriptor
	 */
	[[nodiscard]] int fd() const { return _watchingFd; }

	/**
	 * @brief Active watch event
	 * @param event Interested event
	 */
	void activeEvents(uint32_t event);

	/**
	 * @brief Deactivate watch event
	 * @param event Event to be deactivated
	 */
	void deactivateEvents(uint32_t event);

	void setEvents(uint32_t event);

	/**
	 * @brief Called when interested event happened, dispatch event
	 */
	void handleEvents();

// for Epoll
private:
	/**
	 * @brief Return current activeEvents epoll events
	 */
	[[nodiscard]] inline ::epoll_event watchingEvent() const { return m_watchingEvents; }

	/**
	 * @brief Set activeEvents events, used by poller
	 * 
	 * @param events activeEvents events
	 */
	void setActiveEvents(uint32_t events) { m_activeEvents = events; }

private:
	Epoll *_poll;
	std::weak_ptr<EventHandler> _eventHandler;
	int _watchingFd;
	::epoll_event m_watchingEvents;
	uint32_t m_activeEvents;
};
}

#endif //NETPP_EPOLLEVENT_H
