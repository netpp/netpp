//
// Created by gaojian on 22-5-2.
//

#ifndef NETPP_MOCKEPOLLEVENT_H
#define NETPP_MOCKEPOLLEVENT_H

#include <gmock/gmock.h>
#include "internal/epoll/EpollEvent.h"
#include "MockEpoll.h"

class MockEpollEvent : public netpp::internal::epoll::EpollEvent {
public:
	MockEpollEvent(std::unique_ptr<MockEpoll> mockEpoll = std::unique_ptr<MockEpoll>(new MockEpoll))
	: EpollEvent(
			mockEpoll.get(),
			std::weak_ptr<netpp::internal::epoll::EventHandler>(),
			0)
	{}

	MOCK_METHOD(int, fd, ());
	MOCK_METHOD(void, disable, ());
	MOCK_METHOD(void, activeEvents, (uint32_t));
	MOCK_METHOD(void, deactivateEvents, (uint32_t));
	MOCK_METHOD(void, handleEvents, ());
	MOCK_METHOD(::epoll_event, watchingEvent, ());
	MOCK_METHOD(void, setActiveEvents, (uint32_t));
};
#endif //NETPP_MOCKEPOLLEVENT_H
