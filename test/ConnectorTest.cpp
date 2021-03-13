#include <gtest/gtest.h>
#include "MockSysCallEnvironment.h"
#include <gmock/gmock.h>
#include "handlers/Connector.h"
#include "EventLoopDispatcher.h"

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

TEST_F(ConnectorTest, ConnectRequestTest)
{}

TEST_F(ConnectorTest, StopConnectTest)
{}

ACTION_P(SetArg3ToErrno, value) { *static_cast<int *>(arg3) = value; }

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
