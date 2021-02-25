//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EPOLL_H
#define NETPP_EPOLL_H

#include <vector>
#include <map>
extern "C" {
#include <sys/epoll.h>
}

namespace netpp::internal::epoll {

class EpollEvent;

/**
 * @brief Wrapper of epoll
 * 
 */
class Epoll {
public:
	/**
	 * @brief An Epoll object is a wrapp of epoll, can wait, conrtol epoll events
	 * @throw ResourceLimitException
	 * 1.file descriptors limit
	 * 2.insufficient memory to create the kernel object
	 */
	Epoll();
	~Epoll();

	/**
	 * @brief wait for event once, if nothing ready, returned in 500ms
	 * 
	 * @return a vector of the ready events
	 */
	std::vector<EpollEvent *> poll();

	/**
	 * @brief add or update epoll watching events
	 * 
	 * @param channelEvent	pending update event
	 */
	void updateEvent(EpollEvent *channelEvent);

	/**
	 * @brief remove event from epoll
	 * 
	 * @param channelEvent 
	 */
	void removeEvent(EpollEvent *channelEvent);

private:
	int m_epfd;
	std::vector<::epoll_event> m_activeEvents;
	std::map<int, EpollEvent *> _events;
};
}

#endif //NETPP_EPOLL_H
