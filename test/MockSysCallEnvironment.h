#ifndef MOCKENVIROMENT_H
#define MOCKENVIROMENT_H

#include <gtest/gtest.h>

using EpollCreate = int(*)(int);
using EpollWait = int(*)(int, struct epoll_event *, int, int);
using EpollPWait = int(*)(int, struct epoll_event *, int, int, const sigset_t *);
using EpollCtl = int(*)(int, int, int, struct epoll_event *);
using Close = int(*)(int fd);

class MockSysCallEnvironment : public testing::Environment {
public:
	MockSysCallEnvironment() = default;
	~MockSysCallEnvironment() override = default;

	void SetUp() override;
	void TearDown() override {}

	static void restoreSysCall();

private:
	static EpollCreate real_epoll_create;
	static EpollCreate real_epoll_create1;
	static EpollWait real_epoll_wait;
	static EpollPWait real_epoll_pwait;
	static EpollCtl real_epoll_ctl;
	static Close real_close;
};

extern EpollCreate action_epoll_create;
extern EpollCreate action_epoll_create1;
extern EpollWait action_epoll_wait;
extern EpollPWait action_epoll_pwait;
extern EpollCtl action_epoll_ctl;
extern Close action_close;

#endif
