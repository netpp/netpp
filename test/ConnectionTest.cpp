#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define private public
#define protected public
#include "EventLoop.h"
#include "MockSysCallEnvironment.h"
#include "handlers/TcpConnection.h"
#include "socket/Socket.h"
#include "Address.h"
#include "time/TimeWheel.h"
#undef private
#undef protected
extern "C" {
#include "sys/epoll.h"
}

class EmptyHandler {
public:
	void onConnected([[maybe_unused]] std::shared_ptr<netpp::Channel> channel)
	{
		// ++AcceptorTest::onConnectedCount;
	}
};

class MockConnection : public SysCall {
public:
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));
};

class ConnectionTest : public testing::Test {
public:
protected:
	void SetUp() override
	{
		MockSysCallEnvironment::registerMock(&mock);
	}

	void TearDown() override
	{
		MockSysCallEnvironment::restoreSysCall();
	}

	MockConnection mock;
};

TEST_F(ConnectionTest, CreateConnectionTest)
{
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, GetPtrFromEpollCtl()))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(0)));
	netpp::EventLoop loop;
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, testing::_, EpollEventEq(EPOLLIN | EPOLLERR | EPOLLRDHUP)))
		.Times(1);
	auto connection = netpp::internal::handlers::TcpConnection::makeTcpConnection(
		&loop, std::make_unique<netpp::internal::socket::Socket>(0, netpp::Address()), netpp::Events(std::make_shared<EmptyHandler>())).lock();
	connection->getIOChannel();
	
	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

TEST_F(ConnectionTest, AutoKickTest)
{}

TEST_F(ConnectionTest, RenewWheel)
{
	netpp::EventLoop loop;
	auto wheelPtr = std::make_unique<netpp::internal::time::TimeWheel>(&loop, 2, 2);
	loop.m_kickIdleConnectionWheel = std::move(wheelPtr);
}

TEST_F(ConnectionTest, ConnectionBrokeTest)
{}

TEST_F(ConnectionTest, ManuallyCloseTest)
{}

TEST_F(ConnectionTest, SendTest)
{}

TEST_F(ConnectionTest, RecvTest)
{}
