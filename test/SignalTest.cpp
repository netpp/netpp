#include <gtest/gtest.h>
#include "signal/SignalWatcher.h"
#include "signal/Signals.h"
#include "Events.h"
#include "error/Exception.h"
extern "C" {
#include <signal.h>
}

class SignalHandler {
public:
	void onSignal(netpp::signal::Signals signal)
	{}
};

class SignalTest : public testing::Test {
public:

protected:
	void SetUp() override {}
	void TearDown() override {}
};

// FIXME: signal may catched by gtest threads, use an other process to avoid it
TEST_F(SignalTest, SingleSignalHandle)
{
	// netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_PIPE);
	// netpp::EventLoop *loop = SignalTest::dispatcher->dispatchEventLoop();
	// loop->runInLoop([]{ ::kill(::getpid(), SIGPIPE); });
	// loop->runInLoop([]{ ::kill(::getpid(), SIGALRM); });
	// EXPECT_THROW(loop->run(), netpp::error::UnhandledSignal);
	// netpp::signal::SignalWatcher::restore(netpp::signal::Signals::E_PIPE);
}
