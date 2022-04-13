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
enum class EpollEv {
	/** @brief EPOLLIN event */
	IN		= 0x01,
	/** @brief EPOLLOUT event */
	OUT		= 0x02,
	/** @brief EPOLLRDHUP event */
	RDHUP	= 0x04,
	/**
	 * @brief EPOLLPRI event
	 * @todo out-of-band data are not handled
	 * */
	PRI		= 0x08,
	/** @brief EPOLLERR event */
	ERR		= 0x10,
	/** @brief EPOLLHUP event */
	HUP		= 0x20
};

/**
 * @brief The poll event handler
 */
class EpollEvent {
	friend class Epoll;
public:
	EpollEvent(Epoll *poll, std::weak_ptr<EventHandler> handler, int fd);
	~EpollEvent() = default;

	/**
	 * @brief Get watching fd of this event
	 * @return file descriptor
	 */
	[[nodiscard]] int fd() const { return _watchingFd; }
	
	/**
	 * @brief Remove epoll_event from epoll
	 * @note Must call this to remove event from epoll before destruction
	 */
	void disable();

	/**
	 * @brief Active watch event
	 * @param event Interested event
	 */
	void active(EpollEv event) { active({event}); }

	/**
	 * @brief Deactivate watch event
	 * @param event Event to be deactivated
	 */
	void deactivate(EpollEv event) { deactivate({event}); }

	/**
	 * @brief Active watch serial event
	 * @param events Interested events
	 * @todo use | operator to improve performance
	 */
	void active(std::initializer_list<EpollEv> events);
	/**
	 * @brief Deactivate watch serial event
	 * @param events Events to be deactivated
	 */
	void deactivate(std::initializer_list<EpollEv> events);

	/**
	 * @brief Called when interested event happened, dispatch event
	 */
	void handleEvents();

// for Epoll
private:
	/**
	 * @brief Return current active epoll events
	 */
	[[nodiscard]] inline ::epoll_event watchingEvent() const { return m_watchingEvents; }

	/**
	 * @brief Set active events, used by poller
	 * 
	 * @param events active events
	 */
	void setActiveEvents(uint32_t events) { activeEvents = events; }

private:
	Epoll *_poll;
	std::weak_ptr<EventHandler> _eventHandler;
	int _watchingFd;
	::epoll_event m_watchingEvents;
	uint32_t activeEvents;
};
}

#endif //NETPP_EPOLLEVENT_H
