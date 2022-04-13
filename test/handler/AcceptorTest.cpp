#include <gtest/gtest.h>
#include <gmock/gmock.h>
#define private public
#define protected public
#include "internal/handlers/Acceptor.h"
#include "internal/handlers/RunInLoopHandler.h"
#include "../MockSysCallEnvironment.h"
#include "error/Exception.h"
#include "Address.h"
#include "eventloop/EventLoop.h"
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
TEST_F(AcceptorTest, ListenTest)
{
	netpp::eventloop::EventLoop loop;

	EXPECT_CALL(mock, mock_socket)
		.Times(1);
	auto acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&loop, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
			);

	acceptor->listen();
	loop.m_runInLoop->handleIn();
}

TEST_F(AcceptorTest, StopListenTest)
{
	netpp::EventLoopDispatcher dispatcher(1);
	netpp::EventLoop *loop = dispatcher.dispatchEventLoop();

	EXPECT_CALL(mock, mock_socket)
		.Times(1);
	std::shared_ptr<netpp::internal::handlers::Acceptor> acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
		);

	acceptor->stop();
	acceptor->listen();
	acceptor->stop();

	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, testing::_, testing::_, testing::_))
		.Times(0);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, testing::_, testing::_))
		.Times(1);
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_DEL, testing::_, testing::_))
		.Times(1);
	loop->m_runInLoop->handleIn();

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

TEST_F(AcceptorTest, AcceptConnectionTest)
{
	netpp::EventLoopDispatcher dispatcher(1);
	std::shared_ptr<netpp::internal::handlers::Acceptor> acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
		);
	EXPECT_CALL(mock, mock_accept4);
	acceptor->handleIn();
	EXPECT_EQ(AcceptorTest::onConnectedCount, 1);
}

TEST_F(AcceptorTest, AbortConnectionTest)
{
	netpp::EventLoopDispatcher dispatcher(1);
	std::shared_ptr<netpp::internal::handlers::Acceptor> acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
		);
	EXPECT_CALL(mock, mock_accept4)
		.WillOnce(testing::DoAll(testing::Assign(&errno, ECONNABORTED), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMFILE), testing::Return(-1)));
	acceptor->handleIn();
	EXPECT_EQ(onConnectedCount, 0);
	acceptor->handleIn();
	EXPECT_EQ(onConnectedCount, 0);
}

#pragma GCC diagnostic pop
