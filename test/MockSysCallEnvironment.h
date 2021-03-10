#ifndef MOCKENVIROMENT_H
#define MOCKENVIROMENT_H

#include <gtest/gtest.h>
extern "C" {
#include <unistd.h>
#include <sys/eventfd.h>
#include <arpa/inet.h>
}
struct msghdr;

class SysCall {
public:
	virtual ~SysCall() = default;

	virtual int mock_epoll_create(int size);
	virtual int mock_epoll_create1(int flags);
	virtual int mock_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
	virtual int mock_epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask);
	virtual int mock_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

	virtual int mock_close(int fd);
	virtual ::ssize_t mock_write(int fd, const void *buf, ::size_t count);
	virtual ::ssize_t mock_read(int fd, void *buf, ::size_t count);
	virtual int mock_pipe2(int pipefd[2], int flags);
	virtual ::ssize_t mock_sendmsg(int sockfd, const struct ::msghdr *msg, int flags);
	virtual ::ssize_t mock_recvmsg(int sockfd, struct ::msghdr *msg, int flags);
	virtual int mock_eventfd(unsigned int initval, int flags);
	virtual int mock_eventfd_read(int fd, ::eventfd_t *value);
	virtual int mock_eventfd_write(int fd, ::eventfd_t value);

	virtual int mock_socket(int domain, int type, int protocol);
	virtual int mock_bind(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen);
	virtual int mock_listen(int sockfd, int backlog);
	virtual int mock_accept4(int sockfd, struct ::sockaddr *addr, ::socklen_t *addrlen, int flags);
	virtual int mock_connect(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen);
	virtual int mock_shutdown(int sockfd, int how);
	virtual int mock_getsockopt(int sockfd, int level, int optname, void *optval, ::socklen_t *optlen);

	virtual int mock_timerfd_create(int clockid, int flags);
	virtual int mock_timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
};

using EpollCreate = int(*)(int);
using EpollWait = int(*)(int, struct epoll_event *, int, int);
using EpollPWait = int(*)(int, struct epoll_event *, int, int, const sigset_t *);
using EpollCtl = int(*)(int, int, int, struct epoll_event *);

using Close = int(*)(int);
using Write = ::ssize_t(*)(int, const void *, ::size_t);
using Read = ::ssize_t(*)(int, void *, ::size_t);
using Pipe2 = int(*)(int[2], int);
using SendMsg = ::ssize_t(*)(int, const struct ::msghdr *, int);
using RecvMsg = ::ssize_t(*)(int, struct ::msghdr *, int);
using EventFd = int(*)(unsigned int, int);
using EventFdRead = int(*)(int, ::eventfd_t *);
using EventFdWrite = int(*)(int, ::eventfd_t);

using Socket = int(*)(int, int, int);
using Bind = int(*)(int, const struct ::sockaddr *, ::socklen_t);
using Listen = int(*)(int, int);
using Accept4 = int(*)(int, struct ::sockaddr *, ::socklen_t *, int);
using Connect = int(*)(int, const struct ::sockaddr *, ::socklen_t);
using Shutdown = int(*)(int, int);
using GetSockOpt = int(*)(int, int, int, void *, ::socklen_t *);

using TimerfdCreate = int(*)(int, int);
using TimerfdSettime = int(*)(int, int, const struct itimerspec *, struct itimerspec *);

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
	static Write real_write;
	static Read real_read;
	static Pipe2 real_pipe2;
	static SendMsg real_sendmsg;
	static RecvMsg real_recvmsg;
	static EventFd real_eventfd;
	static EventFdRead real_eventfd_read;
	static EventFdWrite real_eventfd_write;

	static Socket real_socket;
	static Bind real_bind;
	static Listen real_listen;
	static Accept4 real_accept4;
	static Connect real_connect;
	static Shutdown real_shutdown;
	static GetSockOpt real_getsockopt;

	static TimerfdCreate real_timerfd_create;
	static TimerfdSettime real_timerfd_settime;
};

#endif
