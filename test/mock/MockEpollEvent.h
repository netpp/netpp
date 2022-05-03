//
// Created by gaojian on 22-5-2.
//

#ifndef NETPP_MOCKEPOLLEVENT_H
#define NETPP_MOCKEPOLLEVENT_H

#include <gmock/gmock.h>
#include "internal/epoll/EpollEvent.h"

class MockEpollEvent : public netpp::internal::epoll::EpollEvent {
public:
	MockEpollEvent() : EpollEvent(nullptr, std::weak_ptr<netpp::internal::epoll::EventHandler>(), 0) {}
	MOCK_METHOD(int, fd, ());
	MOCK_METHOD(void, disable, ());
	MOCK_METHOD(void, active, (netpp::internal::epoll::EpollEv));
	MOCK_METHOD(void, deactivate, (netpp::internal::epoll::EpollEv));
	MOCK_METHOD(void, active, (std::initializer_list<netpp::internal::epoll::EpollEv>));
	MOCK_METHOD(void, deactivate, (std::initializer_list<netpp::internal::epoll::EpollEv>));
	MOCK_METHOD(void, handleEvents, ());
	MOCK_METHOD(::epoll_event, watchingEvent, ());
	MOCK_METHOD(void, setActiveEvents, (uint32_t));
};
#endif //NETPP_MOCKEPOLLEVENT_H
