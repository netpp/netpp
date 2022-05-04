#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../mock/MockSocket.h"
#include "../mock/MockSysCallEnvironment.h"
#include "../mock/MockEventLoop.h"
#include "../mock/MockEpollEvent.h"
#define private public
#define protected public
#include "internal/handlers/Connector.h"
#include "Address.h"
#include "eventloop/EventLoop.h"
#include "internal/handlers/RunInLoopHandler.h"
#include "error/SocketError.h"
#include "Config.h"
#include "Application.h"
#include "internal/socket/SocketEnums.h"
#undef private
#undef protected

ACTION_P(SetArg3ToErrno, value) { *static_cast<int *>(arg3) = value; }

int onConnectedCount;
netpp::error::SocketError errorCode;
int onErrorCount;

class MockConnector : public SysCall {
public:
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
protected:
	void SetUp() override
	{
		onConnectedCount = 0;
		errorCode = netpp::error::SocketError::E_UNKOWN;
		onErrorCount = 0;
	}

	void TearDown() override
	{
		onConnectedCount = 0;
		errorCode = netpp::error::SocketError::E_UNKOWN;
		onErrorCount = 0;
	}

	MockConnector mock;
};

class EmptyHandler {
public:
	void onConnected(std::shared_ptr<netpp::Channel>)
	{
		++onConnectedCount;
	}

	void onError(netpp::error::SocketError code)
	{
		// todo: test error code
		errorCode = code;
		++onErrorCount;
	}
};

TEST_F(ConnectorTest, CreateConnectorTest)
{
	netpp::eventloop::EventLoop loop;
	EXPECT_CALL(mock, mock_socket)
		.Times(3)
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EACCES), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMFILE), testing::Return(-1)));
	auto connector = netpp::internal::handlers::Connector::makeConnector(
		&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	EXPECT_EQ(connector->m_state, netpp::internal::socket::TcpState::Closed);
	EXPECT_EQ(connector->m_socket->fd(), 1);
	EXPECT_NE(connector, nullptr);
	EXPECT_EQ(onErrorCount, 0);

	connector = netpp::internal::handlers::Connector::makeConnector(
		&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	EXPECT_EQ(connector, nullptr);
	EXPECT_EQ(onErrorCount, 1);

	connector = netpp::internal::handlers::Connector::makeConnector(
		&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	EXPECT_EQ(connector, nullptr);
	EXPECT_EQ(onErrorCount, 2);
}

TEST_F(ConnectorTest, ConnectRequestTest)
{
	MockEventLoop loop;
	auto connector = netpp::internal::handlers::Connector::makeConnector(
			&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	auto socket = std::make_unique<MockSocket>();
	MockSocket *mockSocket = socket.get();
	connector->m_socket = std::move(socket);
	auto epEv = std::make_unique<MockEpollEvent>();
	MockEpollEvent *mockEpEv = epEv.get();
	connector->m_epollEvent = std::move(epEv);

	EXPECT_CALL(loop, runInLoop(RunFunctor())).Times(1);
	EXPECT_CALL(*mockEpEv, activeEvents(netpp::internal::epoll::EpollEv::OUT)).Times(1);
	EXPECT_CALL(*mockSocket, connect);
	connector->connect();
	EXPECT_EQ(connector->m_state, netpp::internal::socket::TcpState::Connecting);
}

TEST_F(ConnectorTest, StopConnectTest)
{
	netpp::Application app;
	app.m_loopManager->m_mainEventLoop = std::make_unique<MockEventLoop>();
	auto *loop = static_cast<MockEventLoop *>(app.m_loopManager->m_mainEventLoop.get());
	auto connector = netpp::internal::handlers::Connector::makeConnector(
			loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	auto socket = std::make_unique<MockSocket>();
	MockSocket *mockSocket = socket.get();
	connector->m_socket = std::move(socket);
	auto epEv = std::make_unique<MockEpollEvent>();
	MockEpollEvent *mockEpEv = epEv.get();
	connector->m_epollEvent = std::move(epEv);

	EXPECT_CALL(*loop, runInLoop(RunFunctor())).Times(0);
	connector->stopConnect();	// do nothing

	EXPECT_CALL(*loop, runInLoop(RunFunctor())).Times(1);
	EXPECT_CALL(*mockEpEv, activeEvents(netpp::internal::epoll::EpollEv::OUT)).Times(1);
	EXPECT_CALL(*mockSocket, connect);
	connector->connect();		// connect
	EXPECT_EQ(connector->m_state, netpp::internal::socket::TcpState::Connecting);

	EXPECT_CALL(*loop, runInLoop(RunFunctor())).Times(1);
	EXPECT_CALL(*mockEpEv, activeEvents(netpp::internal::epoll::EpollEv::OUT)).Times(1);
	EXPECT_CALL(*mockSocket, connect);
	connector->stopConnect();	// not connected yet, stop connect
	EXPECT_EQ(connector->m_state, netpp::internal::socket::TcpState::Connecting);

	EXPECT_CALL(*mockSocket, getError)
		.WillOnce(testing::Return(netpp::error::SocketError::E_NOERROR));
	connector->handleOut();
	EXPECT_EQ(connector->m_state, netpp::internal::socket::TcpState::Established);
	EXPECT_CALL(*loop, runInLoop(RunFunctor())).Times(0);
	connector->stopConnect();	// connected, do nothing
}

TEST_F(ConnectorTest, ConnectFailedTest)
{
	netpp::Application app;
	app.m_loopManager->m_mainEventLoop = std::make_unique<MockEventLoop>();
	auto *loop = static_cast<MockEventLoop *>(app.m_loopManager->m_mainEventLoop.get());
	auto connector = netpp::internal::handlers::Connector::makeConnector(
			loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	connector->m_socket = std::make_unique<MockSocket>();
	auto mockSocket = static_cast<MockSocket *>(connector->m_socket.get());

	EXPECT_CALL(*mockSocket, getError)
			.WillOnce(testing::Return(netpp::error::SocketError::E_INPROGRESS))
		.WillOnce(testing::Return(netpp::error::SocketError::E_TIMEDOUT))
		.WillOnce(testing::Return(netpp::error::SocketError::E_CONNREFUSED))
		.WillOnce(testing::Return(netpp::error::SocketError::E_NOERROR));
	// connecting
	connector->handleOut();
	// retry
	/*connector->handleOut();
	connector->handleOut();
	// connect
	connector->handleOut();*/
}
