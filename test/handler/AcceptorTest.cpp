#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../mock/MockSysCallEnvironment.h"
#include "../mock/MockEpollEvent.h"
#include "../mock/MockEventLoop.h"
#include "../mock/MockSocket.h"
#define private public
#define protected public
#include "internal/handlers/Acceptor.h"
#include "internal/handlers/RunInLoopHandler.h"
#include "error/Exception.h"
#include "Address.h"
#include "internal/socket/SocketEnums.h"
#undef private
#undef protected

class MockAcceptor : public SysCall {
public:
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));

	MOCK_METHOD(int, mock_socket, (int, int, int), (override));
	MOCK_METHOD(int, mock_bind, (int, const struct ::sockaddr *, ::socklen_t), (override));
	MOCK_METHOD(int, mock_listen, (int, int), (override));
	MOCK_METHOD(int, mock_accept4, (int, struct ::sockaddr *, ::socklen_t *, int), (override));
	MOCK_METHOD(int, mock_connect, (int, const struct ::sockaddr *, ::socklen_t), (override));
	MOCK_METHOD(int, mock_shutdown, (int, int), (override));
	MOCK_METHOD(int, mock_getsockopt, (int, int, int, void *, ::socklen_t *), (override));
};

class AcceptorTest : public testing::Test {
public:
	static int onConnectedCount;
protected:
	void SetUp() override
	{
		MockSysCallEnvironment::registerMock(&mock);
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		onConnectedCount = 0;
	}

	void TearDown() override
	{
		MockSysCallEnvironment::restoreSysCall();
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		onConnectedCount = 0;
	}

	MockAcceptor mock;
};
int AcceptorTest::onConnectedCount;

class EmptyHandler {
public:
	void onConnected(std::shared_ptr<netpp::Channel>)
	{
		++AcceptorTest::onConnectedCount;
	}
};

TEST_F(AcceptorTest, CreateTest)
{
	netpp::eventloop::EventLoop loop;
	std::shared_ptr<netpp::internal::handlers::Acceptor> acceptor;
	EXPECT_CALL(mock, mock_socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP))
		.Times(3)
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EACCES), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMFILE), testing::Return(-1)));
	EXPECT_NO_THROW({
		acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig{}
		);}
	);
	EXPECT_NE(acceptor.get(), nullptr);

	EXPECT_NO_THROW({
		acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig{}
		);}
	);
	EXPECT_EQ(acceptor.get(), nullptr);

	EXPECT_NO_THROW({
		acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig{}
		);}
	);
	EXPECT_EQ(acceptor.get(), nullptr);
}

TEST_F(AcceptorTest, ListenTest)
{
	MockEventLoop loop;
	auto acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	auto socket = std::make_unique<MockSocket>();
	MockSocket *mockSocket = socket.get();
	acceptor->m_socket = std::move(socket);
	auto epEv = std::make_unique<MockEpollEvent>();
	MockEpollEvent *mockEpEv = epEv.get();
	acceptor->m_epollEvent = std::move(epEv);

	EXPECT_CALL(loop, runInLoop(RunFunctor())).Times(1);
	EXPECT_CALL(loop, addEventHandlerToLoop).Times(1);
	EXPECT_CALL(*mockEpEv, active(netpp::internal::epoll::EpollEv::OUT)).Times(1);
	EXPECT_CALL(*mockSocket, listen).Times(1);
	acceptor->listen();
	EXPECT_EQ(acceptor->m_state, netpp::internal::socket::TcpState::Listen);
}

TEST_F(AcceptorTest, StopListenTest)
{
	MockEventLoop loop;
	auto acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	auto epEv = std::make_unique<MockEpollEvent>();
	MockEpollEvent *mockEpEv = epEv.get();
	acceptor->m_epollEvent = std::move(epEv);

	EXPECT_CALL(loop, runInLoop(RunFunctor())).Times(1);
	EXPECT_CALL(loop, removeEventHandlerFromLoop).Times(1);
	EXPECT_CALL(*mockEpEv, disable).Times(1);
	acceptor->stop();
	acceptor->listen();
	acceptor->stop();
	EXPECT_EQ(acceptor->m_state, netpp::internal::socket::TcpState::Closed);
}

TEST_F(AcceptorTest, AcceptConnectionTest)
{
	MockEventLoop loop;
	auto acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	auto socket = std::make_unique<MockSocket>();
	MockSocket *mockSocket = socket.get();
	acceptor->m_socket = std::move(socket);

	EXPECT_CALL(*mockSocket, accept);
	acceptor->handleIn();
	EXPECT_EQ(AcceptorTest::onConnectedCount, 1);
}

TEST_F(AcceptorTest, AbortConnectionTest)
{
	MockEventLoop loop;
	auto acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>()), netpp::ConnectionConfig()
	);
	auto socket = std::make_unique<MockSocket>();
	MockSocket *mockSocket = socket.get();
	acceptor->m_socket = std::move(socket);

	EXPECT_CALL(*mockSocket, accept)
		.WillOnce(testing::Throw(netpp::error::SocketException(0)))
		.WillOnce(testing::Throw(netpp::error::SocketException(0)));
	acceptor->handleIn();
	acceptor->handleIn();
	EXPECT_EQ(onConnectedCount, 0);
}
