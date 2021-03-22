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
#include "handlers/RunInLoopHandler.h"
#undef private
#undef protected
extern "C" {
#include "sys/epoll.h"
}

class MockConnection : public SysCall {
public:
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));
	MOCK_METHOD(::ssize_t, mock_recvmsg, (int, struct ::msghdr *, int), (override));
	MOCK_METHOD(::ssize_t, mock_sendmsg, (int, const struct ::msghdr *, int), (override));
	MOCK_METHOD(int, mock_shutdown, (int, int), (override));
};

class ConnectionTest : public testing::Test {
public:
	static int onMessageReceivedCount;
	static int onWriteCompletedCount;
	static int onDisconnectCount;
protected:
	void SetUp() override
	{
		onMessageReceivedCount = 0;
		onWriteCompletedCount = 0;
		onDisconnectCount = 0;
		MockSysCallEnvironment::registerMock(&mock);
	}

	void TearDown() override
	{
		onMessageReceivedCount = 0;
		onWriteCompletedCount = 0;
		onDisconnectCount = 0;
		MockSysCallEnvironment::restoreSysCall();
	}

	MockConnection mock;
};
int ConnectionTest::onMessageReceivedCount;
int ConnectionTest::onWriteCompletedCount;
int ConnectionTest::onDisconnectCount;

class MockHandler {
public:
	void onMessageReceived([[maybe_unused]] std::shared_ptr<netpp::Channel> channel) { ++ConnectionTest::onMessageReceivedCount; }
	void onWriteCompleted() { ++ConnectionTest::onWriteCompletedCount; }
	void onDisconnect() { ++ConnectionTest::onDisconnectCount; }
};

TEST_F(ConnectionTest, CreateConnectionTest)
{
	netpp::EventLoop loop;
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, testing::_, EpollEventEq(EPOLLIN | EPOLLERR | EPOLLRDHUP)))
		.Times(1);
	auto connection = netpp::internal::handlers::TcpConnection::makeTcpConnection(
		&loop, std::make_unique<netpp::internal::socket::Socket>(0, netpp::Address()), netpp::Events(std::make_shared<MockHandler>())).lock();
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
{
	netpp::EventLoop loop;
	auto handler = std::make_shared<MockHandler>();
	auto connection = netpp::internal::handlers::TcpConnection::makeTcpConnection(
			&loop, std::make_unique<netpp::internal::socket::Socket>(0, netpp::Address()), netpp::Events(handler)).lock();
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_DEL, testing::_, testing::_))
		.Times(1);
	connection->handleRdhup();
	EXPECT_EQ(ConnectionTest::onDisconnectCount, 1);

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

TEST_F(ConnectionTest, ReadInBrokenConnectionTest)
{
	netpp::EventLoop loop;
	auto handler = std::make_shared<MockHandler>();
	auto connection = netpp::internal::handlers::TcpConnection::makeTcpConnection(
			&loop, std::make_unique<netpp::internal::socket::Socket>(0, netpp::Address()), netpp::Events(handler)).lock();
	EXPECT_CALL(mock, mock_recvmsg)
		.Times(1)
		.WillOnce(testing::DoAll(testing::Assign(&errno, ECONNREFUSED), testing::Return(-1)));
	connection->handleIn();
	EXPECT_CALL(mock, mock_shutdown(testing::_, SHUT_WR))
			.Times(1);
	loop.m_runInLoop->handleIn();
	connection->handleRdhup();
	EXPECT_EQ(ConnectionTest::onDisconnectCount, 1);

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

TEST_F(ConnectionTest, WriteToBrokenConnectionTest)
{
	netpp::EventLoop loop;
	auto handler = std::make_shared<MockHandler>();
	auto connection = netpp::internal::handlers::TcpConnection::makeTcpConnection(
			&loop, std::make_unique<netpp::internal::socket::Socket>(0, netpp::Address()), netpp::Events(handler)).lock();
	EXPECT_CALL(mock, mock_sendmsg)
			.Times(1)
			.WillOnce(testing::DoAll(testing::Assign(&errno, EPIPE), testing::Return(-1)));
	connection->handleOut();
	EXPECT_CALL(mock, mock_shutdown(testing::_, SHUT_WR))
			.Times(1);
	loop.m_runInLoop->handleIn();
	connection->handleRdhup();
	EXPECT_EQ(ConnectionTest::onDisconnectCount, 1);

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

TEST_F(ConnectionTest, ManuallyCloseTest)
{
	netpp::EventLoop loop;
	auto handler = std::make_shared<MockHandler>();
	auto connection = netpp::internal::handlers::TcpConnection::makeTcpConnection(
			&loop, std::make_unique<netpp::internal::socket::Socket>(0, netpp::Address()), netpp::Events(handler)).lock();
	EXPECT_CALL(mock, mock_shutdown(testing::_, SHUT_WR))
			.Times(0);
	connection->closeAfterWriteCompleted();
	EXPECT_CALL(mock, mock_shutdown(testing::_, SHUT_WR))
			.Times(1);
	loop.m_runInLoop->handleIn();
}

TEST_F(ConnectionTest, DelayCloseTest)
{
	netpp::EventLoop loop;
	auto handler = std::make_shared<MockHandler>();
	auto connection = netpp::internal::handlers::TcpConnection::makeTcpConnection(
			&loop, std::make_unique<netpp::internal::socket::Socket>(0, netpp::Address()), netpp::Events(handler)).lock();
	connection->sendInLoop();
	connection->closeAfterWriteCompleted();
	testing::Sequence s;
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLOUT)))
			.Times(1)
			.InSequence(s);
	EXPECT_CALL(mock, mock_shutdown)
			.Times(0)
			.InSequence(s);
	loop.m_runInLoop->handleIn();

//	connection->closeAfterWriteCompleted();
	EXPECT_CALL(mock, mock_sendmsg)
			.Times(1)
			.InSequence(s);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLIN | EPOLLERR | EPOLLRDHUP)))
			.Times(1)
			.InSequence(s);
	EXPECT_CALL(mock, mock_shutdown(testing::_, SHUT_WR))
			.Times(1)
			.InSequence(s);
	connection->handleOut();
//	loop.m_runInLoop->handleIn();

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

TEST_F(ConnectionTest, SendTest)
{
	netpp::EventLoop loop;
	auto handler = std::make_shared<MockHandler>();
	auto connection = netpp::internal::handlers::TcpConnection::makeTcpConnection(
			&loop, std::make_unique<netpp::internal::socket::Socket>(0, netpp::Address()), netpp::Events(handler)).lock();
	connection->sendInLoop();
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLIN | EPOLLERR | EPOLLRDHUP | EPOLLOUT)))
		.Times(1);
	loop.m_runInLoop->handleIn();
	// write completed, remove EPOLLOUT
	EXPECT_CALL(mock, mock_sendmsg(testing::_, testing::_, MSG_NOSIGNAL))
		.WillOnce(testing::DoAll(testing::Return(0)));
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLIN | EPOLLERR | EPOLLRDHUP)))
			.Times(1);
	connection->handleOut();
	EXPECT_EQ(ConnectionTest::onWriteCompletedCount, 1);

	ConnectionTest::onWriteCompletedCount = 0;
	// not write completed first time
	EXPECT_CALL(mock, mock_sendmsg(testing::_, testing::_, MSG_NOSIGNAL))
			.WillOnce(testing::DoAll(testing::Return(-1)))
			.WillOnce(testing::DoAll(testing::Return(0)));
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_MOD, testing::_, EpollEventEq(EPOLLIN | EPOLLERR | EPOLLRDHUP)))
			.Times(1);
	connection->handleOut();
	connection->handleOut();
	EXPECT_EQ(ConnectionTest::onWriteCompletedCount, 1);

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

TEST_F(ConnectionTest, RecvTest)
{
	netpp::EventLoop loop;
	auto handler = std::make_shared<MockHandler>();
	auto connection = netpp::internal::handlers::TcpConnection::makeTcpConnection(
			&loop, std::make_unique<netpp::internal::socket::Socket>(0, netpp::Address()), netpp::Events(handler)).lock();
	EXPECT_CALL(mock, mock_recvmsg)
			.Times(1);
	connection->handleIn();
	EXPECT_EQ(ConnectionTest::onMessageReceivedCount, 1);
}
