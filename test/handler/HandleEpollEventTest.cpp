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

	epollEvent.m_activeEvents = EPOLLERR | EPOLLRDHUP | EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLHUP;
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

	epollEvent.activeEvents(EpollEv::IN);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLIN);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.activeEvents(EpollEv::OUT);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLOUT);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.activeEvents(EpollEv::RDHUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLRDHUP);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.activeEvents(EpollEv::PRI);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLPRI);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.activeEvents(EpollEv::ERR);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLERR);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.activeEvents(EpollEv::HUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLHUP);
	epollEvent.m_watchingEvents.events = 0;

	epollEvent.activeEvents(EpollEv::IN | EpollEv::OUT | EpollEv::RDHUP | EpollEv::PRI | EpollEv::ERR | EpollEv::HUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLPRI | EPOLLERR | EPOLLHUP);
	epollEvent.m_watchingEvents.events = 0;
}

TEST_F(HandleEpollEventTest, DeactiveEventTest)
{
	Epoll poll;
	auto handler = std::make_shared<MockEventHandler>(nullptr);
	EpollEvent epollEvent(&poll, handler, 0);

	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivateEvents(EpollEv::IN);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivateEvents(EpollEv::OUT);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivateEvents(EpollEv::RDHUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivateEvents(EpollEv::PRI);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivateEvents(EpollEv::ERR);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.deactivateEvents(EpollEv::HUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);

	epollEvent.m_watchingEvents.events = EPOLLERR | EPOLLRDHUP | EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLHUP;
	epollEvent.deactivateEvents(EpollEv::ERR);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLRDHUP | EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLHUP);
	epollEvent.deactivateEvents(EpollEv::RDHUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLHUP);
	epollEvent.deactivateEvents(EpollEv::IN);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLOUT | EPOLLPRI | EPOLLHUP);
	epollEvent.deactivateEvents(EpollEv::OUT);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLPRI | EPOLLHUP);
	epollEvent.deactivateEvents(EpollEv::PRI);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, EPOLLHUP);
	epollEvent.deactivateEvents(EpollEv::HUP);
	EXPECT_EQ(epollEvent.m_watchingEvents.events, 0);
}
