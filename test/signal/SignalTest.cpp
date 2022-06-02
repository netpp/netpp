#include <gtest/gtest.h>
#define private public
#include "signal/Signals.h"
#include "Events.h"
#include "error/Exception.h"
#include "time/Timer.h"
#include "../mock/MockSysCallEnvironment.h"
#include "internal/epoll/EpollEvent.h"
#include "eventloop/EventLoop.h"
#include "internal/epoll/Epoll.h"
#include "Application.h"
#undef private
extern "C" {
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
}

class CustomSignalHandler {
public:
	void onSignal(netpp::signal::Signals signal)
	{
		::exit(netpp::signal::toLinuxSignal(signal));
	}
};

void defaultSignalAction(int bySignal)
{
	netpp::Config config;
	config.signal.enableHandleSignal = false;
	netpp::Application app(config);
	netpp::Events event(std::make_shared<CustomSignalHandler>());
	netpp::time::Timer timer;
	timer.setOnTimeout([bySignal]{
		::kill(::getpid(), bySignal);
	});
	timer.setInterval(15);
	timer.start();
	app.exec();
}

int handleSignal(netpp::signal::Signals bySignal)
{
	netpp::Config config;
	config.tickTimer.enable = false;
	config.signal = {true, {bySignal}};
	config.eventHandler = netpp::Events(std::make_shared<CustomSignalHandler>());
	netpp::Application app(config);
	netpp::time::Timer timer;
	timer.setOnTimeout([bySignal]{
		::kill(::getpid(), netpp::signal::toLinuxSignal(bySignal));
	});
	timer.setInterval(15);
	timer.start();
	app.exec();
	return -1;
}

class SignalTest : public testing::TestWithParam<std::tuple<int, netpp::signal::Signals>> {
public:

};

INSTANTIATE_TEST_SUITE_P(signal_params,
						SignalTest,
						testing::Values(
								std::tuple<int, netpp::signal::Signals>{SIGINT, netpp::signal::Signals::E_INT},
								 std::tuple<int, netpp::signal::Signals>{SIGALRM, netpp::signal::Signals::E_ALRM}
								));

TEST_P(SignalTest, DeadByUnhandledSignal)
{
	int signal = std::get<0>(GetParam());
	EXPECT_EXIT(defaultSignalAction(signal), testing::KilledBySignal(signal), "");
}

TEST_P(SignalTest, DeadByHandledSignal)
{
	int signal = std::get<0>(GetParam());
	netpp::signal::Signals signalEnum = std::get<1>(GetParam());
//	handleSignal(signalEnum);
	EXPECT_EXIT(handleSignal(signalEnum), testing::ExitedWithCode(signal), "");
}
