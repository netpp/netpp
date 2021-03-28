#include <gtest/gtest.h>
#include "internal/epoll/EventHandler.h"
#include "internal/epoll/Epoll.h"
#include "internal/epoll/EpollEvent.h"
#include <memory>
#include <gmock/gmock.h>
#include "MockSysCallEnvironment.h"

class Handler : public netpp::internal::epoll::EventHandler {
	friend class EpollEventTest;
public:
	Handler() = default;
	~Handler() override = default;

	MOCK_METHOD(void, handleIn, (), (override));
	MOCK_METHOD(void, handleOut, (), (override));
	MOCK_METHOD(void, handleErr, (), (override));
	MOCK_METHOD(void, handleRdhup, (), (override));
	MOCK_METHOD(void, handlePri, (), (override));
	MOCK_METHOD(void, handleHup, (), (override));
};

class EpollMock : public SysCall {
public:
	EpollMock() = default;
	~EpollMock() override = default;

	MOCK_METHOD(int, mock_epoll_create, (int), (override));
	MOCK_METHOD(int, mock_epoll_create1, (int), (override));
	MOCK_METHOD(int, mock_epoll_wait, (int, struct epoll_event *, int, int), (override));
	MOCK_METHOD(int, mock_epoll_pwait, (int, struct epoll_event *, int, int, const sigset_t *), (override));
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));
	MOCK_METHOD(int, mock_close, (int), (override));
};

class EpollEventTest : public testing::Test {
protected:
	void SetUp() override
	{
		MockSysCallEnvironment::registerMock(&mock);
		epoll = std::make_unique<netpp::internal::epoll::Epoll>();
		handler = std::make_shared<Handler>();
		auto epollEvent = std::make_unique<netpp::internal::epoll::EpollEvent>(epoll.get(), handler, 0);
		event = epollEvent.get();
		handler->m_epollEvent = std::move(epollEvent);
	}

	void TearDown() override
	{
		MockSysCallEnvironment::restoreSysCall();
	}
	
	static void SetUpTestCase()
	{}

	static void TearDownTestCase() {}

	EpollMock mock;

	std::shared_ptr<Handler> handler;
	netpp::internal::epoll::EpollEvent *event;
	std::unique_ptr<netpp::internal::epoll::Epoll> epoll;
};

TEST_F(EpollEventTest, ActiveAndDeactiveEvent)
{
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, testing::_, EpollEventEq(EPOLLERR)))
		.Times(1);
	event->active(netpp::internal::epoll::Event::ERR);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLERR | EPOLLHUP)))
		.Times(1);
	event->active(netpp::internal::epoll::Event::HUP);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLERR | EPOLLHUP | EPOLLIN)))
		.Times(1);
	event->active(netpp::internal::epoll::Event::IN);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLERR | EPOLLHUP | EPOLLIN | EPOLLOUT)))
		.Times(1);
	event->active(netpp::internal::epoll::Event::OUT);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLERR | EPOLLHUP | EPOLLIN | EPOLLOUT | EPOLLPRI)))
		.Times(1);
	event->active(netpp::internal::epoll::Event::PRI);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLERR | EPOLLHUP | EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLRDHUP)))
		.Times(1);
	event->active(netpp::internal::epoll::Event::RDHUP);

	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLHUP | EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLRDHUP)))
		.Times(1);
	event->deactive(netpp::internal::epoll::Event::ERR);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLIN | EPOLLOUT | EPOLLPRI | EPOLLRDHUP)))
		.Times(1);
	event->deactive(netpp::internal::epoll::Event::HUP);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLOUT | EPOLLPRI | EPOLLRDHUP)))
		.Times(1);
	event->deactive(netpp::internal::epoll::Event::IN);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLPRI | EPOLLRDHUP)))
		.Times(1);
	event->deactive(netpp::internal::epoll::Event::OUT);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLRDHUP)))
		.Times(1);
	event->deactive(netpp::internal::epoll::Event::PRI);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(0u)))
		.Times(1);
	event->deactive(netpp::internal::epoll::Event::RDHUP);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(0u)))
		.Times(1);
	event->deactive(netpp::internal::epoll::Event::RDHUP);
}

TEST_F(EpollEventTest, RemoveEvent)
{
	// not add to epoll by default
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_DEL, testing::_, testing::_))
		.Times(0);
	event->disable();

	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, testing::_, EpollEventEq(EPOLLERR)))
		.Times(1);
	event->active(netpp::internal::epoll::Event::ERR);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_DEL, testing::_, testing::_))
		.Times(1);
	event->disable();

	// remove again
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_DEL, testing::_, testing::_))
		.Times(0);
	event->disable();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
TEST_F(EpollEventTest, PollEvent)
{
	::epoll_event ev[1];
	ev[0].data.ptr = static_cast<void *>(event);
	EXPECT_CALL(mock, mock_epoll_wait(testing::_, testing::_, testing::_, testing::_))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLERR), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLRDHUP), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLIN), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLOUT), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLPRI), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLHUP), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLIN | EPOLLOUT), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)));
	event->active(netpp::internal::epoll::Event::ERR);
	Handler &h = *handler;

	EXPECT_CALL(h, handleErr).Times(1);
	EXPECT_CALL(h, handleRdhup).Times(0);
	EXPECT_CALL(h, handleIn).Times(0);
	EXPECT_CALL(h, handleOut).Times(0);
	EXPECT_CALL(h, handlePri).Times(0);
	EXPECT_CALL(h, handleHup).Times(0);
	std::vector<netpp::internal::epoll::EpollEvent *> activeChannels{4};
	epoll->poll(activeChannels);
	event->handleEvents();

	EXPECT_CALL(h, handleErr).Times(0);
	EXPECT_CALL(h, handleRdhup).Times(1);
	EXPECT_CALL(h, handleIn).Times(0);
	EXPECT_CALL(h, handleOut).Times(0);
	EXPECT_CALL(h, handlePri).Times(0);
	EXPECT_CALL(h, handleHup).Times(0);
	epoll->poll(activeChannels);
	event->handleEvents();

	EXPECT_CALL(h, handleErr).Times(0);
	EXPECT_CALL(h, handleRdhup).Times(0);
	EXPECT_CALL(h, handleIn).Times(1);
	EXPECT_CALL(h, handleOut).Times(0);
	EXPECT_CALL(h, handlePri).Times(0);
	EXPECT_CALL(h, handleHup).Times(0);
	epoll->poll(activeChannels);
	event->handleEvents();

	EXPECT_CALL(h, handleErr).Times(0);
	EXPECT_CALL(h, handleRdhup).Times(0);
	EXPECT_CALL(h, handleIn).Times(0);
	EXPECT_CALL(h, handleOut).Times(1);
	EXPECT_CALL(h, handlePri).Times(0);
	EXPECT_CALL(h, handleHup).Times(0);
	epoll->poll(activeChannels);
	event->handleEvents();

	EXPECT_CALL(h, handleErr).Times(0);
	EXPECT_CALL(h, handleRdhup).Times(0);
	EXPECT_CALL(h, handleIn).Times(0);
	EXPECT_CALL(h, handleOut).Times(0);
	EXPECT_CALL(h, handlePri).Times(1);
	EXPECT_CALL(h, handleHup).Times(0);
	epoll->poll(activeChannels);
	event->handleEvents();

	EXPECT_CALL(h, handleErr).Times(0);
	EXPECT_CALL(h, handleRdhup).Times(0);
	EXPECT_CALL(h, handleIn).Times(0);
	EXPECT_CALL(h, handleOut).Times(0);
	EXPECT_CALL(h, handlePri).Times(0);
	EXPECT_CALL(h, handleHup).Times(1);
	epoll->poll(activeChannels);
	event->handleEvents();

	EXPECT_CALL(h, handleErr).Times(0);
	EXPECT_CALL(h, handleRdhup).Times(0);
	EXPECT_CALL(h, handleIn).Times(1);
	EXPECT_CALL(h, handleOut).Times(1);
	EXPECT_CALL(h, handlePri).Times(0);
	EXPECT_CALL(h, handleHup).Times(0);
	epoll->poll(activeChannels);
	event->handleEvents();
}

#pragma GCC diagnostic pop
