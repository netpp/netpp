//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EPOLL_H
#define NETPP_EPOLL_H

#include <vector>
#include <map>

struct epoll_event;

namespace netpp {
class EpollEventHandler;
// TODO: support et mode
/**
 * @brief Wrapper of epoll
 * 
 */
class Epoll {
public:
	/**
	 * @brief An Epoll object is a wrapper of epoll, can wait, control epoll events
	 * @throw ResourceLimitException
	 * 1.file descriptors limit
	 * 2.insufficient memory to create the kernel object
	 */
	Epoll();
	// TODO: define with Rule-of-Three(effective modern c++ item 17)
	~Epoll();

	/**
	 * @brief Wait for event once, and take care of events
	 */
	void poll();

	void addEvent(EpollEventHandler *eventHandler);
	/**
	 * @brief Add or update epoll watching events
	 * 
	 * @param channelEvent	Pending update event
	 */
	void updateEvent(EpollEventHandler *eventHandler);

	/**
	 * @brief Remove event from epoll
	 * 
	 * @param channelEvent 
	 */
	void removeEvent(EpollEventHandler *eventHandler);

private:
	int realCtl(int epfd, int op, int fd, struct ::epoll_event *event);

	int m_epollFd;
	std::vector<::epoll_event> m_activeEvents;
};
}

#endif //NETPP_EPOLL_H
