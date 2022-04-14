//
// Created by gaojian on 2022/4/13.
//

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../MockSysCallEnvironment.h"

class MockTimer : public SysCall {
public:
	MOCK_METHOD(int, mock_signalfd, (int, const sigset_t *, int), (override));
	MOCK_METHOD(int, mock_sigaddset, (sigset_t *, int), (override));
	MOCK_METHOD(int, mock_pthread_sigmask, (int, const sigset_t *, sigset_t *), (override));
};

class SignalHandlerTest : public testing::Test {
public:
	void SetUp() override {}
	void TearDown() override {}
};
