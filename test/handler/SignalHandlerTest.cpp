//
// Created by gaojian on 2022/4/13.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../mock/MockSysCallEnvironment.h"
#include "eventloop/EventLoop.h"
#include "signal/Signals.h"
extern "C" {
#include <sys/signalfd.h>
#include <csignal>
}
#define private public
#define protected public
#include "internal/handlers/SignalHandler.h"
#undef private
#undef protected

#define SUPPORTED_SIGNAL_DEF(type) SIG##type,
static const std::vector<int> signalSet = {
		NETPP_SINGAL(SUPPORTED_SIGNAL_DEF)
};
#undef SUPPORTED_SIGNAL_DEF

MATCHER_P(SignalSetEq, sets, "")
{
	if (arg)
	{
		if (std::ranges::all_of(signalSet.cbegin(), signalSet.cend(),
								[&](int s){
			return ::sigismember(sets, s) == ::sigismember(arg, s);
		}))
			return true;
		return false;
	}
	return false;
}

class MockTimer : public SysCall {
public:
	MOCK_METHOD(int, mock_signalfd, (int, const sigset_t *, int), (override));
	MOCK_METHOD(int, mock_sigaddset, (sigset_t *, int), (override));
	MOCK_METHOD(int, mock_sigdelset, (sigset_t *, int), (override));
	MOCK_METHOD(int, mock_pthread_sigmask, (int, const sigset_t *, sigset_t *), (override));

	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));
	MOCK_METHOD(int, mock_close, (int), (override));
	MOCK_METHOD(::ssize_t, mock_read, (int, void *, ::size_t), (override));
};

netpp::signal::Signals lastSignal = netpp::signal::Signals::E_HUP;
int signalCbCount = 0;

class SignalCb {
public:
	void onSignal(netpp::signal::Signals signal)
	{
		++signalCbCount;
		lastSignal = signal;
	}
};

class SignalHandlerTest : public testing::Test {
public:
	void SetUp() override
	{
		lastSignal = netpp::signal::Signals::E_HUP;
		signalCbCount = 0;
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		MockSysCallEnvironment::registerMock(&mock);
	}

	void TearDown() override
	{
		lastSignal = netpp::signal::Signals::E_HUP;
		signalCbCount = 0;
		MockSysCallEnvironment::ptrFromEpollCtl = nullptr;
		MockSysCallEnvironment::restoreSysCall();
	}

	MockTimer mock;
};

TEST_F(SignalHandlerTest, CreateHandlerTest)
{
	int fakeSignalFd = 1;
	netpp::eventloop::EventLoop loop;
	::sigset_t defaultSignalSet{};
	::sigemptyset(&defaultSignalSet);
	EXPECT_CALL(mock, mock_signalfd(-1, SignalSetEq(&defaultSignalSet), SFD_NONBLOCK | SFD_CLOEXEC))
			.Times(1)
			.WillOnce(testing::DoAll(testing::Return(fakeSignalFd)));
	EXPECT_CALL(mock, mock_epoll_ctl(testing::_, EPOLL_CTL_ADD, fakeSignalFd, EpollEventEq(EPOLLIN)))
			.Times(1);
	auto handler = netpp::internal::handlers::SignalHandler::makeSignalHandler(&loop, netpp::Events(std::make_shared<SignalCb>()));

	EXPECT_CALL(mock, mock_close(fakeSignalFd))
			.Times(1);
	EXPECT_CALL(mock, mock_pthread_sigmask(SIG_SETMASK, SignalSetEq(&defaultSignalSet), testing::_))
			.Times(1);
}

TEST_F(SignalHandlerTest, RegisterSignalsTest)
{
	int fakeSignalFd = 1;
	netpp::eventloop::EventLoop loop;
	EXPECT_CALL(mock, mock_signalfd(testing::_, testing::_, testing::_))
			.WillOnce(testing::DoAll(testing::Return(fakeSignalFd)));
	auto handler = netpp::internal::handlers::SignalHandler::makeSignalHandler(&loop, netpp::Events(std::make_shared<SignalCb>()));

	EXPECT_CALL(mock, mock_sigaddset(testing::_, SIGABRT))
		.Times(1);
	EXPECT_CALL(mock, mock_signalfd(fakeSignalFd, testing::_, SFD_NONBLOCK | SFD_CLOEXEC))
		.Times(1);
	EXPECT_CALL(mock, mock_pthread_sigmask(SIG_SETMASK, testing::_, testing::_))
			.Times(1);
	handler->startWatchSignal(netpp::signal::Signals::E_ABRT);

	EXPECT_CALL(mock, mock_sigdelset(testing::_, SIGABRT))
			.Times(1);
	EXPECT_CALL(mock, mock_signalfd(fakeSignalFd, testing::_, SFD_NONBLOCK | SFD_CLOEXEC))
			.Times(1);
	EXPECT_CALL(mock, mock_pthread_sigmask(SIG_SETMASK, testing::_, testing::_))
			.Times(1);
	handler->stopWatchSignal(netpp::signal::Signals::E_ABRT);
}

ACTION_P(SetArg1ToSig, value, size)
{
	auto arg = static_cast<::signalfd_siginfo *>(arg1);
	for (int i = 0; i < size; ++i)
		arg[i] = value[i];
}

TEST_F(SignalHandlerTest, HandleSignalTest)
{
	int fakeSignalFd = 1;
	netpp::eventloop::EventLoop loop;
	EXPECT_CALL(mock, mock_signalfd(testing::_, testing::_, testing::_))
			.WillOnce(testing::DoAll(testing::Return(fakeSignalFd)));
	auto handler = netpp::internal::handlers::SignalHandler::makeSignalHandler(&loop, netpp::Events(std::make_shared<SignalCb>()));

	handler->startWatchSignal(netpp::signal::Signals::E_ABRT);

	::signalfd_siginfo signalEmit[1];
	signalEmit[0].ssi_signo = SIGABRT;
	EXPECT_CALL(mock, mock_read(fakeSignalFd, testing::_, testing::_))
		.Times(1)
		.WillOnce(testing::DoAll(SetArg1ToSig(signalEmit, static_cast<int>(1)), testing::Return(sizeof(::signalfd_siginfo))));
	handler->handleIn();
	EXPECT_EQ(lastSignal, netpp::signal::Signals::E_ABRT);
	EXPECT_EQ(signalCbCount, 1);
}

TEST_F(SignalHandlerTest, HandleMultipleSignalTest)
{
	int fakeSignalFd = 1;
	netpp::eventloop::EventLoop loop;
	EXPECT_CALL(mock, mock_signalfd(testing::_, testing::_, testing::_))
			.WillOnce(testing::DoAll(testing::Return(fakeSignalFd)));
	auto handler = netpp::internal::handlers::SignalHandler::makeSignalHandler(&loop, netpp::Events(std::make_shared<SignalCb>()));

	handler->startWatchSignal(netpp::signal::Signals::E_ABRT);

	constexpr int siginfoSize = 30;
	::signalfd_siginfo signalEmit[siginfoSize];
	signalEmit[0].ssi_signo = SIGABRT;
	for (unsigned i = 0; i < siginfoSize && i < signalSet.size(); ++i)
		signalEmit[i].ssi_signo = static_cast<uint32_t>(signalSet.at(i));
	EXPECT_CALL(mock, mock_read(fakeSignalFd, testing::_, testing::_))
			.Times(1)
			.WillOnce(testing::DoAll(SetArg1ToSig(signalEmit, static_cast<int>(signalSet.size())), testing::Return(sizeof(::signalfd_siginfo))));
	handler->handleIn();
	EXPECT_EQ(lastSignal, netpp::signal::Signals::E_ABRT);
	EXPECT_EQ(signalCbCount, 20);
}
