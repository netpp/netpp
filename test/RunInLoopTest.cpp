#include <gtest/gtest.h>
#include "EventLoop.h"
#include "MockSysCallEnvironment.h"
#include <gmock/gmock.h>
#include "internal/epoll/Epoll.h"
#include "internal/handlers/RunInLoopHandler.h"
#include "internal/epoll/EpollEvent.h"

class RunInLoopMock : public SysCall {
public:
	RunInLoopMock() = default;
	~RunInLoopMock() override = default;

	// MOCK_METHOD(int, mock_epoll_create, (int), (override));
	// MOCK_METHOD(int, mock_epoll_create1, (int), (override));
	MOCK_METHOD(int, mock_epoll_wait, (int, struct epoll_event *, int, int), (override));
	// MOCK_METHOD(int, mock_epoll_pwait, (int, struct epoll_event *, int, int, const sigset_t *), (override));
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));
	// MOCK_METHOD(int, mock_close, (int), (override));
	
	MOCK_METHOD(int, mock_eventfd, (unsigned int, int), (override));
	MOCK_METHOD(int, mock_eventfd_read, (int, ::eventfd_t *), (override));
	MOCK_METHOD(int, mock_eventfd_write, (int, ::eventfd_t), (override));
};

class RunInLoopTest : public testing::Test {
protected:
	void SetUp() override
	{
		runInLoopCount = 0;
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		MockSysCallEnvironment::registerMock(&mock);
	}

	void TearDown() override
	{
		runInLoopCount = 0;
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		MockSysCallEnvironment::restoreSysCall();
	}

	RunInLoopMock mock;
public:
	static int runInLoopCount;
};
int RunInLoopTest::runInLoopCount = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
TEST_F(RunInLoopTest, CreateRunInLoopTest)
{
	EXPECT_CALL(mock, mock_eventfd).Times(1);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, GetPtrFromEpollCtl()))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(0)));
	netpp::EventLoop eventLoop;
	ASSERT_NE(MockSysCallEnvironment::ptrFromEpollCtl, nullptr);

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, nullptr))
		.Times(1);
}

TEST_F(RunInLoopTest, RunFunctorInLoopTest)
{
	EXPECT_CALL(mock, mock_eventfd).Times(1);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, GetPtrFromEpollCtl()))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(0)));
	netpp::EventLoop eventLoop;
	
	EXPECT_CALL(mock, mock_eventfd_write).Times(1);
	eventLoop.runInLoop([&]{
		++runInLoopCount;
	});

	netpp::internal::epoll::EpollEvent *epollEvent = static_cast<netpp::internal::epoll::EpollEvent *>(MockSysCallEnvironment::ptrFromEpollCtl);
	ASSERT_NE(epollEvent, nullptr);
	netpp::internal::epoll::Epoll *epoll = eventLoop.getPoll();
	::epoll_event ev[1];
	ev[0].data.ptr = static_cast<void *>(epollEvent);
	EXPECT_CALL(mock, mock_epoll_wait(testing::_, testing::_, testing::_, testing::_))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLIN), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)));
	std::vector<netpp::internal::epoll::EpollEvent *> activeChannels{4};
	epoll->poll(activeChannels);

	epollEvent->handleEvents();
	EXPECT_EQ(runInLoopCount, 1);

	EXPECT_CALL(mock, mock_eventfd_write).Times(1);
	eventLoop.runInLoop([&]{
		++runInLoopCount;
	});
	EXPECT_CALL(mock, mock_eventfd_write).Times(1);
	eventLoop.runInLoop([&]{
		++runInLoopCount;
	});
	EXPECT_CALL(mock, mock_epoll_wait(testing::_, testing::_, testing::_, testing::_))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLIN), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)));
	epoll->poll(activeChannels);

	epollEvent->handleEvents();
	EXPECT_EQ(runInLoopCount, 3);

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, nullptr))
		.Times(1);
}

#pragma GCC diagnostic pop
