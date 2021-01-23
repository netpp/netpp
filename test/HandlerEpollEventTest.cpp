#include <gtest/gtest.h>
#include "epoll/EventHandler.h"
#include "epoll/Epoll.h"
#include "epoll/EpollEvent.h"
#include <memory>
extern "C" {
#include <dlfcn.h>
}

class Handler : public netpp::epoll::EventHandler {
	friend class EpollEventTest;
public:
	Handler()
		: EventHandler(0), readTriggerCount{0}, writeTriggerCount{0}, errorTriggerCount{0}, disconnectTriggerCount{0}
	{}
	~Handler() override = default;

	void handleRead() noexcept override
	{ ++readTriggerCount; }
	void handleWrite() noexcept override
	{ ++writeTriggerCount; }
	void handleError() noexcept override
	{ ++errorTriggerCount; }
	void handleClose() noexcept override
	{ ++disconnectTriggerCount; }

	int readTriggerCount;
	int writeTriggerCount;
	int errorTriggerCount;
	int disconnectTriggerCount;

	void setEpollEvent(std::unique_ptr<netpp::epoll::EpollEvent> &&event) { m_epollEvent = std::move(event); }
	netpp::epoll::EpollEvent *getEpollEvent() { return m_epollEvent.get(); }
};

class EpollEventTest : public testing::Test {
protected:
	void SetUp() override
	{
		epoll = std::make_unique<netpp::epoll::Epoll>();
		handler = std::make_shared<Handler>();
		handler->setEpollEvent(std::make_unique<netpp::epoll::EpollEvent>(epoll.get(), handler));
		event = handler->getEpollEvent();
	}
	void TearDown() override {}
	
	static void SetUpTestCase()
	{
		::dlsym(RTLD_NEXT, "epoll_create");
		::dlsym(RTLD_NEXT, "epoll_create1");
		::dlsym(RTLD_NEXT, "epoll_wait");
		::dlsym(RTLD_NEXT, "epoll_pwait");
		::dlsym(RTLD_NEXT, "epoll_ctl");
		::dlsym(RTLD_NEXT, "close");
	}

	static void TearDownTestCase() {}

	std::shared_ptr<Handler> handler;
	netpp::epoll::EpollEvent *event;
	std::unique_ptr<netpp::epoll::Epoll> epoll;
};

extern "C" {
int epoll_create(int size)
{
	(void)size;
	return 1;
}

int epoll_create1(int flags)
{
	(void)flags;
	return 1;
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	(void)epfd;
	(void)events;
	(void)maxevents;
	(void)timeout;
	return 0;
}

int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask)
{
	(void)epfd;
	(void)events;
	(void)maxevents;
	(void)timeout;
	(void)sigmask;
	return 0;
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	(void)epfd;
	(void)op;
	(void)fd;
	(void)event;
	return 0;
}

int close(int fd)
{
	(void)fd;
	return 0;
}
}

TEST_F(EpollEventTest, AddEvent)
{
	// TODO test this
}

TEST_F(EpollEventTest, RemoveEvent)
{
	// TODO test this
}
