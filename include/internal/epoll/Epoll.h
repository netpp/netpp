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
	 * @brief Wait for event once
	 * 
	 * @return A vector of the ready events
	 */
	std::vector<internal::epoll::EpollEvent *>::size_type poll(std::vector<internal::epoll::EpollEvent *> &channels);

	/**
	 * @brief Add or update epoll watching events
	 * 
	 * @param channelEvent	Pending update event
	 */
	void updateEvent(EpollEvent *channelEvent);

	/**
	 * @brief Remove event from epoll
	 * 
	 * @param channelEvent 
	 */
	void removeEvent(EpollEvent *channelEvent);

private:
	int m_epollFd;
	std::vector<::epoll_event> m_activeEvents;
	std::map<int, EpollEvent *> _events;
};
}

#endif //NETPP_EPOLL_H
