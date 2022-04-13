//
// Created by gaojian on 2022/4/11.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#define private public
#define protected public
#include "internal/epoll/EventHandler.h"
#include "internal/epoll/Epoll.h"
#undef private
#undef protected

using namespace netpp::internal::epoll;

class MockEventHandler : public EventHandler {
public:
	explicit MockEventHandler(netpp::eventloop::EventLoop *loop) : EventHandler(loop) {}
	~MockEventHandler() override = default;
	MOCK_METHOD(void, handleIn, (), (override));
	MOCK_METHOD(void, handleOut, (), (override));
	MOCK_METHOD(void, handleErr, (), (override));
	MOCK_METHOD(void, handleRdhup, (), (override));
	MOCK_METHOD(void, handlePri, (), (override));
	MOCK_METHOD(void, handleHup, (), (override));
};

class HandleEpollEventTest : public testing::Test {
public:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(HandleEpollEventTest, HandleEventTest)
{
	Epoll poll;
	auto handler = std::make_shared<MockEventHandler>(nullptr);
	EpollEvent epollEvent(&poll, handler, 0);

	testing::InSequence s;

	epollEvent.activeEvents = EPOLLERR | EPOLLRDHUP | EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLHUP;
	EXPECT_CALL(*handler, handleErr);
	EXPECT_CALL(*handler, handleRdhup);
	EXPECT_CALL(*handler, handleIn);
	EXPECT_CALL(*handler, handleOut);
	EXPECT_CALL(*handler, handlePri);
	EXPECT_CALL(*handler, handleHup);
	epollEvent.handleEvents();
}

TEST_F(HandleEpollEventTest, ActiveEventTest)
{
	Epoll poll;
	auto handler = std::make_shared<MockEventHandler>(nullptr);
	EpollEvent epollEvent(&poll, handler, 0);

	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.active(EpollEv::IN);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLIN);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.active(EpollEv::OUT);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLOUT);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.active(EpollEv::RDHUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLRDHUP);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.active(EpollEv::PRI);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLPRI);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.active(EpollEv::ERR);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLERR);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.active(EpollEv::HUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLHUP);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.active({ EpollEv::HUP });
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLHUP);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.active({ EpollEv::IN, EpollEv::OUT, EpollEv::RDHUP, EpollEv::PRI, EpollEv::ERR, EpollEv::HUP});
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP);
	epollEvent.m_watchingEvents.events = 0;
}

TEST_F(HandleEpollEventTest, DeactiveEventTest)
{
	Epoll poll;
	auto handler = std::make_shared<MockEventHandler>(nullptr);
	EpollEvent epollEvent(&poll, handler, 0);

	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivate(EpollEv::IN);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivate(EpollEv::OUT);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivate(EpollEv::RDHUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivate(EpollEv::PRI);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivate(EpollEv::ERR);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivate(EpollEv::HUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.m_watchingEvents.events = EPOLLERR | EPOLLRDHUP | EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLHUP;
	epollEvent.deactivate(EpollEv::ERR);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLRDHUP | EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLHUP);
	epollEvent.deactivate(EpollEv::RDHUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLHUP);
	epollEvent.deactivate(EpollEv::IN);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLOUT | EPOLLPRI | EPOLLHUP);
	epollEvent.deactivate(EpollEv::OUT);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLPRI | EPOLLHUP);
	epollEvent.deactivate(EpollEv::PRI);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLHUP);
	epollEvent.deactivate(EpollEv::HUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);
}
