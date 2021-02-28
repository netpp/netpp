#include <gtest/gtest.h>
#include "epoll/EventHandler.h"
#include "epoll/Epoll.h"
#include "epoll/EpollEvent.h"
#include <memory>

class Handler : public netpp::internal::epoll::EventHandler {
	friend class EpollEventTest;
public:
	Handler()
		// : readTriggerCount{0}, writeTriggerCount{0}, errorTriggerCount{0}, disconnectTriggerCount{0}
	{}
	~Handler() override = default;

	// void handleRead() noexcept override
	// { ++readTriggerCount; }
	// void handleWrite() noexcept override
	// { ++writeTriggerCount; }
	// void handleError() noexcept override
	// { ++errorTriggerCount; }
	// void handleClose() noexcept override
	// { ++disconnectTriggerCount; }

	// int readTriggerCount;
	// int writeTriggerCount;
	// int errorTriggerCount;
	// int disconnectTriggerCount;
};

class EpollEventTest : public testing::Test {
protected:
	void SetUp() override
	{
		epoll = std::make_unique<netpp::internal::epoll::Epoll>();
		handler = std::make_shared<Handler>();
		auto epollEvent = std::make_unique<netpp::internal::epoll::EpollEvent>(epoll.get(), handler, 0);
		event = epollEvent.get();
		handler->m_epollEvent = std::move(epollEvent);
	}
	void TearDown() override {}
	
	static void SetUpTestCase()
	{}

	static void TearDownTestCase() {}

	std::shared_ptr<Handler> handler;
	netpp::internal::epoll::EpollEvent *event;
	std::unique_ptr<netpp::internal::epoll::Epoll> epoll;
};

TEST_F(EpollEventTest, AddEvent)
{
	// TODO: test this
}

TEST_F(EpollEventTest, RemoveEvent)
{
	// TODO: test this
}
