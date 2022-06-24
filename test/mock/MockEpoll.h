//
// Created by gaojian on 22-5-2.
//


#ifndef NETPP_MOCKEPOLL_H
#define NETPP_MOCKEPOLL_H

#include <gmock/gmock.h>
#include "epoll/Epoll.h"

class MockEpoll : public netpp::internal::epoll::Epoll {
public:
	MOCK_METHOD(std::vector<netpp::internal::epoll::EpollEvent *>::size_type, fd, (std::vector<netpp::internal::epoll::EpollEvent *> &));

	MOCK_METHOD(void, addEvent, (netpp::internal::epoll::EpollEvent *));
	MOCK_METHOD(void, updateEvent, (netpp::internal::epoll::EpollEvent *));
	MOCK_METHOD(void, removeEvent, (netpp::internal::epoll::EpollEvent *));
};

#endif //NETPP_MOCKEPOLL_H
