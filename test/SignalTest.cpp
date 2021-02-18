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
	void onSignal(netpp::signal::Signals signal)
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
	try
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
		timer2.setInterval(15);
		timer2.start();

		dispatcher.startLoop();
	}
	catch (netpp::error::UnhandledSignal &us)
	{
		::exit(netpp::signal::toLinuxSignal(us.signal()));
	}
}

class SignalTest : public testing::Test {
public:
protected:
	static void SetUpTestCase()
	{
	}

	static void TearDownTestCase()
	{
	}

	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(SignalTest, HandleSignalInOtherProccess)
{
	EXPECT_EXIT(killSelf(), testing::KilledBySignal(SIGALRM), "");
	// EXPECT_DEBUG_DEATH(killSelfWithSignalWatcher(), "");
	EXPECT_EXIT(killSelfWithSignalWatcher(), testing::ExitedWithCode(SIGQUIT), "");
}
