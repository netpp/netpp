#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define private public
#define protected public
#include "MockSysCallEnvironment.h"
#include "handlers/Connector.h"
#include "EventLoopDispatcher.h"
#include "Address.h"
#include "EventLoop.h"
#undef private
#undef protected

ACTION_P(SetArg3ToErrno, value) { *static_cast<int *>(arg3) = value; }

class MockConnector : public SysCall {
public:
	MOCK_METHOD(int, mock_epoll_wait, (int, struct epoll_event *, int, int), (override));
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));

	MOCK_METHOD(int, mock_socket, (int, int, int), (override));
	MOCK_METHOD(int, mock_bind, (int, const struct ::sockaddr *, ::socklen_t), (override));
	MOCK_METHOD(int, mock_listen, (int, int), (override));
	MOCK_METHOD(int, mock_accept4, (int, struct ::sockaddr *, ::socklen_t *, int), (override));
	MOCK_METHOD(int, mock_connect, (int, const struct ::sockaddr *, ::socklen_t), (override));
	MOCK_METHOD(int, mock_shutdown, (int, int), (override));
	MOCK_METHOD(int, mock_getsockopt, (int, int, int, void *, ::socklen_t *), (override));

	MOCK_METHOD(int, mock_timerfd_create, (int, int), (override));
	MOCK_METHOD(int, mock_timerfd_settime, (int, int, const struct itimerspec *, struct itimerspec *), (override));

	MOCK_METHOD(int, mock_eventfd_write, (int, ::eventfd_t), (override));
};

class ConnectorTest : public testing::Test {
public:
	static int onConnectedCount;
protected:
	void SetUp() override
	{
		MockSysCallEnvironment::registerMock(&mock);
		onConnectedCount = 0;
	}

	void TearDown() override
	{
		MockSysCallEnvironment::restoreSysCall();
		onConnectedCount = 0;
	}

	MockConnector mock;
};
int ConnectorTest::onConnectedCount;

class EmptyHandler {
public:
	void onConnected([[maybe_unused]] std::shared_ptr<netpp::Channel> channel)
	{
		++ConnectorTest::onConnectedCount;
	}
};

TEST_F(ConnectorTest, CreateConnectorTest)
{
	netpp::EventLoopDispatcher dispatcher(1);
	EXPECT_CALL(mock, mock_socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP))
		.Times(3)
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EACCES), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMFILE), testing::Return(-1)));
	auto connector = netpp::internal::handlers::Connector::makeConnector(
		&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
	);
	EXPECT_NE(connector, nullptr);

	connector = netpp::internal::handlers::Connector::makeConnector(
		&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
	);
	EXPECT_EQ(connector, nullptr);

	connector = netpp::internal::handlers::Connector::makeConnector(
		&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
	);
	EXPECT_EQ(connector, nullptr);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
TEST_F(ConnectorTest, ConnectRequestTest)
{
	// get run in loop
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, GetPtrFromEpollCtl()))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(0)));
		
	netpp::EventLoopDispatcher dispatcher(1);
	netpp::internal::epoll::EpollEvent *epollEvent = static_cast<netpp::internal::epoll::EpollEvent *>(MockSysCallEnvironment::ptrFromEpollCtl);
	ASSERT_NE(epollEvent, nullptr);
	::epoll_event ev[1];
	ev[0].data.ptr = static_cast<void *>(epollEvent);
	netpp::EventLoop *loop = dispatcher.dispatchEventLoop();
	netpp::internal::epoll::Epoll *epoll = loop->getPoll();
	EXPECT_CALL(mock, mock_socket)
		.Times(1);
	std::shared_ptr<netpp::internal::handlers::Connector> connector = netpp::internal::handlers::Connector::makeConnector(
			&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
		);
	
	EXPECT_CALL(mock, mock_eventfd_write)
		.Times(1);
	connector->connect();

	EXPECT_CALL(mock, mock_epoll_wait)
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLIN), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)));
	epoll->poll();
	EXPECT_CALL(mock, mock_epoll_ctl).Times(1);
	EXPECT_CALL(mock, mock_connect).Times(1);
	epollEvent->handleEvents();

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

TEST_F(ConnectorTest, StopConnectTest)
{
	// get run in loop
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, GetPtrFromEpollCtl()))
		.Times(1)
		.WillOnce(testing::DoAll(testing::Return(0)));
		
	netpp::EventLoopDispatcher dispatcher(1);
	netpp::internal::epoll::EpollEvent *epollEvent = static_cast<netpp::internal::epoll::EpollEvent *>(MockSysCallEnvironment::ptrFromEpollCtl);
	ASSERT_NE(epollEvent, nullptr);
	::epoll_event ev[1];
	ev[0].data.ptr = static_cast<void *>(epollEvent);
	netpp::EventLoop *loop = dispatcher.dispatchEventLoop();
	netpp::internal::epoll::Epoll *epoll = loop->getPoll();
	EXPECT_CALL(mock, mock_socket)
		.Times(1);
	std::shared_ptr<netpp::internal::handlers::Connector> connector = netpp::internal::handlers::Connector::makeConnector(
			&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
		);
	
	EXPECT_CALL(mock, mock_eventfd_write)
		.Times(3);
	connector->stopConnect();	// do nothing
	connector->connect();		// connect
	connector->stopConnect();	// not connected yet, stop connect

	EXPECT_CALL(mock, mock_epoll_wait)
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLIN), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)));
	epoll->poll();
	testing::Sequence s1;
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, testing::_, testing::_))
		.Times(1)
		.InSequence(s1);
	EXPECT_CALL(mock, mock_connect)
		.Times(1)
		.InSequence(s1);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_DEL, testing::_, nullptr))
		.Times(1)
		.InSequence(s1);
	epollEvent->handleEvents();

	EXPECT_CALL(mock, mock_getsockopt)
		.WillOnce(testing::DoAll(SetArg3ToErrno(0), testing::Return(0)));
	EXPECT_CALL(mock, mock_epoll_ctl)
		.Times(1);
	connector->handleOut();
	EXPECT_CALL(mock, mock_eventfd_write)
		.Times(1);
	connector->stopConnect();	// connected, do nothing
	EXPECT_CALL(mock, mock_epoll_wait)
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLIN), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)));
	epoll->poll();
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_DEL, testing::_, nullptr))
		.Times(0);
	epollEvent->handleEvents();

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

#pragma GCC diagnostic pop

TEST_F(ConnectorTest, ConnectTest)
{
	netpp::EventLoopDispatcher dispatcher(1);
	auto connector = netpp::internal::handlers::Connector::makeConnector(
		&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
	);
	EXPECT_CALL(mock, mock_getsockopt)
		.Times(4)
		.WillOnce(testing::DoAll(SetArg3ToErrno(EINPROGRESS), testing::Return(0)))
		.WillOnce(testing::DoAll(SetArg3ToErrno(ETIMEDOUT), testing::Return(0)))
		.WillOnce(testing::DoAll(SetArg3ToErrno(ECONNREFUSED), testing::Return(0)))
		.WillOnce(testing::DoAll(SetArg3ToErrno(0), testing::Return(0)));
	
	// connecting
	connector->handleOut();
	// retry
	EXPECT_CALL(mock, mock_timerfd_settime).Times(4);
	connector->handleOut();
	connector->handleOut();
	// connect
	connector->handleOut();
}
