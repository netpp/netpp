#ifndef MOCKENVIROMENT_H
#define MOCKENVIROMENT_H

#include <gtest/gtest.h>

using EpollCreate = int(*)(int);
using EpollWait = int(*)(int, struct epoll_event *, int, int);
using EpollPWait = int(*)(int, struct epoll_event *, int, int, const sigset_t *);
using EpollCtl = int(*)(int, int, int, struct epoll_event *);
using Close = int(*)(int);

class SysCall {
public:
	virtual ~SysCall() = default;

	virtual int mock_epoll_create(int size);
	virtual int mock_epoll_create1(int flags);
	virtual int mock_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
	virtual int mock_epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask);
	virtual int mock_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
	virtual int mock_close(int fd);
};

class MockSysCallEnvironment : public testing::Environment {
public:
	MockSysCallEnvironment() = default;
	~MockSysCallEnvironment() override = default;

	void SetUp() override;
	void TearDown() override {}

	static void registerMock(SysCall *mock);
	static void restoreSysCall();

public:
	static SysCall *defaultSysMock;
	static SysCall *sysMock;

	static EpollCreate real_epoll_create;
	static EpollCreate real_epoll_create1;
	static EpollWait real_epoll_wait;
	static EpollPWait real_epoll_pwait;
	static EpollCtl real_epoll_ctl;
	static Close real_close;
};

#endif
