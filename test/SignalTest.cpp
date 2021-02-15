#include <gtest/gtest.h>
#include "signal/SignalWatcher.h"
#include "signal/Signals.h"
#include "Events.h"
#include "error/Exception.h"
#include "SignalTest.h"
#include "EventLoopDispatcher.h"
#include "error/Exception.h"
extern "C" {
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
}

class SignalHandler {
public:
	void onSignal(netpp::signal::Signals signal)
	{}
};

void SignalProcess::prepareSignalProcess()
{
	if (::pipe(pip) == -1)
	{
	}

	pid = ::fork();
	if (pid == 0)	// child
	{
		try
		{
			::close(pip[1]);
			netpp::EventLoopDispatcher dispatcher;
			netpp::Events event(std::make_shared<SignalHandler>());
			netpp::signal::SignalWatcher::enableWatchSignal(&dispatcher, std::move(event));
			netpp::signal::SignalWatcher::watch(netpp::signal::Signals::E_ALRM);
			dispatcher.startLoop();
		}
		catch (netpp::error::UnhandledSignal &us)
		{
			::exit(netpp::signal::toLinuxSignal(us.signal()));
		}
	}
	else if (pid > 0)	// parent
	{
		::close(pip[0]);
	}
	else			// failed
	{

	}
}

int SignalProcess::pip[2];
int SignalProcess::pid;

class SignalTest : public testing::Test {
public:

protected:
	static void SetUpTestCase()
	{
		// ignore pipe signal
		struct ::sigaction action;
		action.sa_handler = SIG_IGN;
		::sigaction(SIGPIPE, &action, nullptr);
	}

	static void TearDownTestCase()
	{
		// ignore pipe signal
		struct ::sigaction action;
		action.sa_handler = SIG_DFL;
		::sigaction(SIGPIPE, &action, nullptr);
	}

	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(SignalTest, HandleSignalInOtherProccess)
{
	// if child process died, write will send a SIGPIPE signal
	char writeData;
	::write(SignalProcess::pip[1], &writeData, sizeof(char));
	EXPECT_NE(errno, EPIPE);
	::kill(SignalProcess::pid, SIGALRM);
	::write(SignalProcess::pip[1], &writeData, sizeof(char));
	EXPECT_NE(errno, EPIPE);
	::kill(SignalProcess::pid, SIGSEGV);
	int status;
	EXPECT_EQ(::waitpid(SignalProcess::pid, &status, 0), SignalProcess::pid);
	int isExit = WIFEXITED(status);
	EXPECT_TRUE(isExit);
	int sigNum = WEXITSTATUS(status);
	EXPECT_EQ(SIGSEGV, sigNum);
}
