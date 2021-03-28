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
	IN		= 0x01,
	OUT		= 0x02,
	RDHUP	= 0x04,
	PRI		= 0x08,	// TODO: out-of-band data are not handled
	ERR		= 0x10,
	HUP		= 0x20
};

class EpollEvent {
	friend class Epoll;
public:
	EpollEvent(Epoll *poll, std::weak_ptr<EventHandler> handler, int fd);
	~EpollEvent() = default;

	[[nodiscard]] int fd() const { return _watchingFd; }
	
	/**
	 * @brief Remove epoll_event from epoll
	 * @note Must call this to remove event from epoll before destruction
	 */
	void disable();

	void active(EpollEv event) { active({event}); }
	void deactive(EpollEv event) { deactive({event}); }
	void active(std::initializer_list<EpollEv> events);
	void deactive(std::initializer_list<EpollEv> events);

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
