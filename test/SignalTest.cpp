#include <gtest/gtest.h>
#define private public
#include "signal/SignalWatcher.h"
#include "signal/Signals.h"
#include "Events.h"
#include "error/Exception.h"
#include "EventLoopDispatcher.h"
#include "time/Timer.h"
#include "MockSysCallEnvironment.h"
#include "internal/epoll/EpollEvent.h"
#include "EventLoop.h"
#include "internal/epoll/Epoll.h"
#undef private
extern "C" {
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
}

class SignalHandler {
public:
	void onSignal([[maybe_unused]] netpp::signal::Signals signal)
	{}
};

void killSelf()
{
	netpp::EventLoopDispatcher dispatcher;
	netpp::Events event(std::make_shared<SignalHandler>());
	auto loop = dispatcher.dispatchEventLoop();
	netpp::time::Timer timer(loop);
	timer.setOnTimeout([]{
		::kill(::getpid(), SIGALRM);
	});
	timer.setInterval(15);
	timer.start();
	dispatcher.startLoop();
}

void killSelfWithSignalWatcher()
{
	netpp::signal::SignalWatcher::enableWatchSignal();

	netpp::EventLoopDispatcher dispatcher;
	netpp::Events event(std::make_shared<SignalHandler>());
	netpp::signal::SignalWatcher::with(&dispatcher, std::move(event))
								.watch(netpp::signal::Signals::E_ALRM);

	auto loop1 = dispatcher.dispatchEventLoop();
	netpp::time::Timer timer1(loop1);
	timer1.setOnTimeout([]{
		::kill(::getpid(), SIGALRM);
	});
	timer1.setInterval(15);
	timer1.start();

	auto loop2 = dispatcher.dispatchEventLoop();
	netpp::time::Timer timer2(loop2);
	timer2.setOnTimeout([]{
		::kill(::getpid(), SIGQUIT);
	});
	timer2.setInterval(30);
	timer2.start();

	dispatcher.startLoop();
}

class SignalDeathTest : public testing::Test {
public:
protected:
	static void SetUpTestCase() {}
	static void TearDownTestCase() {}

	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(SignalDeathTest, NotHandleSignal)
{
	EXPECT_EXIT(killSelf(), testing::KilledBySignal(SIGALRM), "");
}

TEST_F(SignalDeathTest, HandleSignal)
{
	EXPECT_EXIT(killSelfWithSignalWatcher(), testing::KilledBySignal(SIGQUIT), "");
}

class SignalTest : public testing::Test {
public:
protected:
	static void SetUpTestCase() {}
	static void TearDownTestCase() {}

	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(SignalTest, ConvertSignal)
{
	EXPECT_EQ(SIGHUP, netpp::signal::toLinuxSignal(netpp::signal::Signals::E_HUP));
	EXPECT_EQ(SIGPWR, netpp::signal::toLinuxSignal(netpp::signal::Signals::E_PWR));
	EXPECT_EQ(SIGSYS, netpp::signal::toLinuxSignal(netpp::signal::Signals::E_SYS));

	EXPECT_EQ(netpp::signal::Signals::E_HUP, netpp::signal::toNetppSignal(SIGHUP));
	EXPECT_EQ(netpp::signal::Signals::E_PWR, netpp::signal::toNetppSignal(SIGPWR));
	EXPECT_EQ(netpp::signal::Signals::E_SYS, netpp::signal::toNetppSignal(SIGSYS));

	EXPECT_NE(std::string(), netpp::signal::signalAsString(netpp::signal::Signals::E_HUP));
	EXPECT_NE(std::string(), netpp::signal::signalAsString(netpp::signal::Signals::E_PWR));
	EXPECT_NE(std::string(), netpp::signal::signalAsString(netpp::signal::Signals::E_SYS));
	EXPECT_NE(std::string(), netpp::signal::signalAsString(SIGHUP));
	EXPECT_NE(std::string(), netpp::signal::signalAsString(SIGPWR));
	EXPECT_NE(std::string(), netpp::signal::signalAsString(SIGSYS));
}

TEST_F(SignalTest, SignalWatchStatus)
{
	EXPECT_EQ(false, netpp::signal::SignalWatcher::isWatching(netpp::signal::Signals::E_HUP));
	EXPECT_EQ(false, netpp::signal::SignalWatcher::isWatching(netpp::signal::Signals::E_PWR));
	EXPECT_EQ(false, netpp::signal::SignalWatcher::isWatching(netpp::signal::Signals::E_SYS));
	EXPECT_EQ(false, netpp::signal::SignalWatcher::isWatching(SIGHUP));
	EXPECT_EQ(false, netpp::signal::SignalWatcher::isWatching(SIGPWR));
	EXPECT_EQ(false, netpp::signal::SignalWatcher::isWatching(SIGSYS));

	// watch signal is not enabled
	netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_PWR);
	EXPECT_EQ(false, netpp::signal::SignalWatcher::isWatching(netpp::signal::Signals::E_PWR));
}

class MockSignal : public SysCall {
public:
	MOCK_METHOD(int, mock_sigaddset, (sigset_t *, int), (override));
	MOCK_METHOD(int, mock_signalfd, (int, const sigset_t *, int), (override));
	MOCK_METHOD(int, mock_sigdelset, (sigset_t *, int), (override));
	MOCK_METHOD(int, mock_sigismember, (const sigset_t *, int), (override));
	MOCK_METHOD(int, mock_sigemptyset, (sigset_t *), (override));
	MOCK_METHOD(int, mock_sigfillset, (sigset_t *), (override));
	MOCK_METHOD(int, mock_pthread_sigmask, (int, const sigset_t *, sigset_t *), (override));

	MOCK_METHOD(int, mock_epoll_wait, (int, struct epoll_event *, int, int), (override));
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));
};

class SigEvent {};

class SignalMockTest : public testing::Test {
protected:
	void SetUp() override
	{
		MockSysCallEnvironment::registerMock(&mock);
	}

	void TearDown() override
	{
		MockSysCallEnvironment::restoreSysCall();
	}

	static void TearDownTestCase()
	{
		netpp::signal::SignalWatcher::signalFd = -1;
	}

	MockSignal mock;
};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Waddress-of-packed-member"
TEST_F(SignalMockTest, EnableHandleSignalTest)
{
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

	EXPECT_CALL(mock, mock_pthread_sigmask)
		.Times(1);
	EXPECT_CALL(mock, mock_sigfillset);
	EXPECT_CALL(mock, mock_sigemptyset);
	EXPECT_CALL(mock, mock_signalfd)
		.WillOnce(testing::Return(-1));
	netpp::signal::SignalWatcher::enableWatchSignal();

	netpp::signal::SignalWatcher::with(&dispatcher, netpp::Events(std::make_shared<SigEvent>()));
	EXPECT_CALL(mock, mock_epoll_wait(testing::_, testing::_, testing::_, testing::_))
		.WillOnce(testing::DoAll(testing::Assign(&ev[0].events, EPOLLIN), testing::SetArrayArgument<1>(ev, ev + 1), testing::Return(1)));
	std::vector<netpp::internal::epoll::EpollEvent *> activeChannels{4};
	epoll->poll(activeChannels);
	EXPECT_CALL(mock, mock_epoll_ctl).Times(1);
	epollEvent->handleEvents();

	// destruction
	EXPECT_CALL(mock, mock_epoll_ctl);
}

#pragma GCC diagnostic pop

TEST_F(SignalMockTest, AddSignalTest)
{
	netpp::signal::SignalWatcher::signalFd = 1;

	EXPECT_CALL(mock, mock_signalfd)
		.Times(3);
	EXPECT_CALL(mock, mock_sigaddset(testing::_, SIGABRT));
	netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_ABRT);
	EXPECT_CALL(mock, mock_sigaddset(testing::_, SIGALRM));
	netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_ALRM);
	EXPECT_CALL(mock, mock_sigaddset(testing::_, SIGCHLD));
	netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_CHLD);
}

TEST_F(SignalMockTest, DelSignalTest)
{
	netpp::signal::SignalWatcher::signalFd = 1;

	EXPECT_CALL(mock, mock_signalfd)
		.Times(3);
	EXPECT_CALL(mock, mock_sigdelset(testing::_, SIGABRT));
	netpp::signal::SignalWatcher::restore(netpp::signal::Signals::E_ABRT);
	EXPECT_CALL(mock, mock_sigdelset(testing::_, SIGALRM));
	netpp::signal::SignalWatcher::restore(netpp::signal::Signals::E_ALRM);
	EXPECT_CALL(mock, mock_sigdelset(testing::_, SIGCHLD));
	netpp::signal::SignalWatcher::restore(netpp::signal::Signals::E_CHLD);
}
