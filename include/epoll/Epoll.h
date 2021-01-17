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

namespace netpp::epoll {

class EpollEvent;

class Epoll {
public:
	Epoll();
	~Epoll();

	std::vector<EpollEvent *> poll();
	void updateEvent(EpollEvent *channelEvent);
	void removeEvent(EpollEvent *channelEvent);

private:
	int m_epfd;
	std::vector<::epoll_event> m_activeEvents;
	std::map<int, EpollEvent *> _events;
};
}

#endif //NETPP_EPOLL_H
