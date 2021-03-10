#include <gtest/gtest.h>
#include "handlers/Acceptor.h"
#include "EventLoopDispatcher.h"

class AcceptorTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

class EmptyHandler {};

TEST_F(AcceptorTest, AcceptorCreateTest)
{
	netpp::EventLoopDispatcher dispatcher;
	std::shared_ptr<netpp::internal::handlers::Acceptor> acceptor;
	EXPECT_NO_THROW(
	{
		acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
		);
	});
	EXPECT_NE(acceptor.get(), nullptr);
}

TEST_F(AcceptorTest, AcceptorListenTest)
{
	netpp::EventLoopDispatcher dispatcher(1);
	netpp::EventLoop *loop = dispatcher.dispatchEventLoop();
	netpp::internal::epoll::Epoll *epoll = loop->getPoll();
	std::shared_ptr<netpp::internal::handlers::Acceptor> acceptor = netpp::internal::handlers::Acceptor::makeAcceptor(
			&dispatcher, netpp::Address(), netpp::Events(std::make_shared<EmptyHandler>())
		);
	acceptor->listen();
	epoll->poll();
}

TEST_F(AcceptorTest, AcceptorStopListenTest)
{}

TEST_F(AcceptorTest, AcceptorIncommingConnectionTest)
{}
