#include <gtest/gtest.h>
#include "eventloop/EventLoop.h"
#include "../mock/MockSysCallEnvironment.h"
#include <gmock/gmock.h>
#include "internal/epoll/Epoll.h"
#define private public
#define protected public
#include "internal/handlers/RunInLoopHandler.h"
#undef private
#undef protected

class RunInLoopMock : public SysCall {
public:
	RunInLoopMock() = default;
	~RunInLoopMock() override = default;

	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));
	MOCK_METHOD(int, mock_close, (int), (override));
	
	MOCK_METHOD(int, mock_eventfd, (unsigned int, int), (override));
	MOCK_METHOD(int, mock_eventfd_read, (int, ::eventfd_t *), (override));
	MOCK_METHOD(int, mock_eventfd_write, (int, ::eventfd_t), (override));
};

class RunInLoopTest : public testing::Test {
protected:
	void SetUp() override
	{
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		MockSysCallEnvironment::registerMock(&mock);
	}

	void TearDown() override
	{
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		MockSysCallEnvironment::restoreSysCall();
	}

	RunInLoopMock mock;
public:
};

TEST_F(RunInLoopTest, CreateRunInLoopTest)
{
	int fakeEventFd = 1;
	EXPECT_CALL(mock, mock_eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(fakeEventFd)));
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, fakeEventFd, EpollEventEq(EPOLLIN)))
		.Times(1);
	netpp::eventloop::EventLoop eventLoop;
	auto handler = netpp::internal::handlers::RunInLoopHandler::makeRunInLoopHandler(&eventLoop);
	EXPECT_EQ(handler->m_wakeUpFd, fakeEventFd);

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, nullptr))
		.Times(1);
	EXPECT_CALL(mock, mock_close(fakeEventFd))
		.Times(1);
}

TEST_F(RunInLoopTest, AddFunctorTest)
{
	int fakeEventFd = 1;
	netpp::eventloop::EventLoop eventLoop;
	EXPECT_CALL(mock, mock_eventfd)
			.WillOnce(testing::DoAll(testing::Return(fakeEventFd)));
	auto handler = netpp::internal::handlers::RunInLoopHandler::makeRunInLoopHandler(&eventLoop);
	EXPECT_EQ(handler->m_pendingFunctors.size(), 0);
	EXPECT_CALL(mock, mock_eventfd_write(fakeEventFd, 1)).Times(1);
	handler->addPendingFunction([]{});
	EXPECT_EQ(handler->m_pendingFunctors.size(), 1);
}

TEST_F(RunInLoopTest, RunFunctorInLoopTest)
{
	int fakeEventFd = 1;
	netpp::eventloop::EventLoop eventLoop;
	EXPECT_CALL(mock, mock_eventfd)
			.WillOnce(testing::DoAll(testing::Return(fakeEventFd)));
	auto handler = netpp::internal::handlers::RunInLoopHandler::makeRunInLoopHandler(&eventLoop);
	int count = 0;
	auto task = [&count]{ ++count; };
	handler->addPendingFunction(task);
	EXPECT_CALL(mock, mock_eventfd_read(fakeEventFd, testing::_)).Times(1);
	handler->handleIn();
	EXPECT_EQ(handler->m_pendingFunctors.size(), 0);
	EXPECT_EQ(count, 1);

	count = 0;
	handler->addPendingFunction(task);
	handler->addPendingFunction(task);
	handler->addPendingFunction(task);
	handler->addPendingFunction(task);
	handler->addPendingFunction(task);
	EXPECT_CALL(mock, mock_eventfd_read(fakeEventFd, testing::_)).Times(1);
	handler->handleIn();
	EXPECT_EQ(handler->m_pendingFunctors.size(), 0);
	EXPECT_EQ(count, 5);
}
