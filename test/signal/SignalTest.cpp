#include <gtest/gtest.h>
#include "signal/Signals.h"
#include "error/Exception.h"
#include "time/Timer.h"
#include "eventloop/EventLoop.h"
#include "epoll/Epoll.h"
#include "Application.h"
extern "C" {
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
}

void defaultSignalAction(int bySignal)
{
	netpp::Application app;
	netpp::Timer timer;
	timer.setOnTimeout([bySignal]{
		::kill(::getpid(), bySignal);
	});
	timer.setInterval(15);
	timer.start();
	app.exec();
}

int handleSignal(netpp::Signals bySignal)
{
	netpp::Application app;
	app.bindSignalHandler({bySignal}, [](netpp::Signals signal){
		::exit(netpp::toLinuxSignal(signal));
	});
	netpp::Timer timer;
	timer.setOnTimeout([bySignal]{
		::kill(::getpid(), netpp::toLinuxSignal(bySignal));
	});
	timer.setInterval(15);
	timer.start();
	app.exec();
	return -1;
}

class SignalTest : public testing::TestWithParam<std::tuple<int, netpp::Signals>> {
public:

};

INSTANTIATE_TEST_SUITE_P(signal_params,
						SignalTest,
						testing::Values(
								std::tuple<int, netpp::Signals>{SIGALRM, netpp::Signals::E_ALRM},
								std::tuple<int, netpp::Signals>{SIGINT, netpp::Signals::E_INT}
								));

TEST_P(SignalTest, DeadByUnhandledSignal)
{
//	int signal = std::get<0>(GetParam());
//	EXPECT_EXIT(defaultSignalAction(signal), testing::KilledBySignal(signal), "");
}

TEST_P(SignalTest, DeadByHandledSignal)
{
//	int signal = std::get<0>(GetParam());
//	netpp::Signals signalEnum = std::get<1>(GetParam());
//	handleSignal(signalEnum);
//	EXPECT_EXIT(handleSignal(signalEnum), testing::ExitedWithCode(signal), "");
}

TEST_P(SignalTest, DeadBySignal)
{
	EXPECT_EXIT(defaultSignalAction(SIGALRM), testing::KilledBySignal(SIGALRM), "");
	EXPECT_EXIT(defaultSignalAction(SIGINT), testing::KilledBySignal(SIGINT), "");
	EXPECT_EXIT(handleSignal(netpp::Signals::E_ALRM), testing::ExitedWithCode(SIGALRM), "");
	EXPECT_EXIT(handleSignal(netpp::Signals::E_INT), testing::ExitedWithCode(SIGINT), "");
}