#include <gtest/gtest.h>
#include "signal/SignalWatcher.h"
#include "signal/Signals.h"
#include "Events.h"
#include "error/Exception.h"
#include "EventLoopDispatcher.h"
#include "error/Exception.h"
#include "time/Timer.h"
extern "C" {
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>
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
