#include "MockSysCallEnvironment.h"
extern "C" {
#include <dlfcn.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
}

int SysCall::mock_epoll_create(int size)
{ return MockSysCallEnvironment::real_epoll_create(size); }

int SysCall::mock_epoll_create1(int flags)
{ return MockSysCallEnvironment::real_epoll_create1(flags); }
	
int SysCall::mock_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{ return MockSysCallEnvironment::real_epoll_wait(epfd, events, maxevents, timeout); }

int SysCall::mock_epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask)
{ return MockSysCallEnvironment::real_epoll_pwait(epfd, events, maxevents, timeout, sigmask); }

int SysCall::mock_epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{ return MockSysCallEnvironment::real_epoll_ctl(epfd, op, fd, event); }

int SysCall::mock_close(int fd)
{ return MockSysCallEnvironment::real_close(fd); }

::ssize_t SysCall::mock_write(int fd, const void *buf, ::size_t count)
{ return MockSysCallEnvironment::real_write(fd, buf, count); }

::ssize_t SysCall::mock_read(int fd, void *buf, ::size_t count)
{ return MockSysCallEnvironment::real_read(fd, buf, count); }

int SysCall::mock_pipe2(int pipefd[2], int flags)
{ return MockSysCallEnvironment::real_pipe2(pipefd, flags); }

::ssize_t SysCall::mock_sendmsg(int sockfd, const struct ::msghdr *msg, int flags)
{ return MockSysCallEnvironment::real_sendmsg(sockfd, msg, flags); }

::ssize_t SysCall::mock_recvmsg(int sockfd, struct ::msghdr *msg, int flags)
{ return MockSysCallEnvironment::real_recvmsg(sockfd, msg, flags); }

int SysCall::mock_eventfd(unsigned int initval, int flags)
{ return MockSysCallEnvironment::real_eventfd(initval, flags); }

int SysCall::mock_eventfd_read(int fd, ::eventfd_t *value)
{ return MockSysCallEnvironment::real_eventfd_read(fd, value); }

int SysCall::mock_eventfd_write(int fd, ::eventfd_t value)
{ return MockSysCallEnvironment::real_eventfd_write(fd, value); }

int SysCall::mock_socket(int domain, int type, int protocol)
{ return MockSysCallEnvironment::real_socket(domain, type, protocol); }

int SysCall::mock_bind(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen)
{ return MockSysCallEnvironment::real_bind(sockfd, addr, addrlen); }

int SysCall::mock_listen(int sockfd, int backlog)
{ return MockSysCallEnvironment::real_listen(sockfd, backlog); }

int SysCall::mock_accept4(int sockfd, struct ::sockaddr *addr, ::socklen_t *addrlen, int flags)
{ return MockSysCallEnvironment::real_accept4(sockfd, addr, addrlen, flags); }

int SysCall::mock_connect(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen)
{ return MockSysCallEnvironment::real_connect(sockfd, addr, addrlen); }

int SysCall::mock_shutdown(int sockfd, int how)
{ return MockSysCallEnvironment::real_shutdown(sockfd, how); }

int SysCall::mock_getsockopt(int sockfd, int level, int optname, void *optval, ::socklen_t *optlen)
{ return MockSysCallEnvironment::real_getsockopt(sockfd, level, optname, optval, optlen); }

SysCall *MockSysCallEnvironment::defaultSysMock;
SysCall *MockSysCallEnvironment::sysMock;

EpollCreate MockSysCallEnvironment::real_epoll_create;
EpollCreate MockSysCallEnvironment::real_epoll_create1;
EpollWait MockSysCallEnvironment::real_epoll_wait;
EpollPWait MockSysCallEnvironment::real_epoll_pwait;
EpollCtl MockSysCallEnvironment::real_epoll_ctl;
Close MockSysCallEnvironment::real_close;
Write MockSysCallEnvironment::real_write;
Read MockSysCallEnvironment::real_read;
Pipe2 MockSysCallEnvironment::real_pipe2;
SendMsg MockSysCallEnvironment::real_sendmsg;
RecvMsg MockSysCallEnvironment::real_recvmsg;
EventFd MockSysCallEnvironment::real_eventfd;
EventFdRead MockSysCallEnvironment::real_eventfd_read;
EventFdWrite MockSysCallEnvironment::real_eventfd_write;

Socket MockSysCallEnvironment::real_socket;
Bind MockSysCallEnvironment::real_bind;
Listen MockSysCallEnvironment::real_listen;
Accept4 MockSysCallEnvironment::real_accept4;
Connect MockSysCallEnvironment::real_connect;
Shutdown MockSysCallEnvironment::real_shutdown;
GetSockOpt MockSysCallEnvironment::real_getsockopt;

void MockSysCallEnvironment::SetUp()
{
	defaultSysMock = new SysCall();
	sysMock = defaultSysMock;
	// compiler will use custome defined functions firstly 
	real_epoll_create = reinterpret_cast<EpollCreate>(::dlsym(RTLD_NEXT, "epoll_create"));
	real_epoll_create1 = reinterpret_cast<EpollCreate>(::dlsym(RTLD_NEXT, "epoll_create1"));
	real_epoll_wait = reinterpret_cast<EpollWait>(::dlsym(RTLD_NEXT, "epoll_wait"));
	real_epoll_pwait = reinterpret_cast<EpollPWait>(::dlsym(RTLD_NEXT, "epoll_pwait"));
	real_epoll_ctl = reinterpret_cast<EpollCtl>(::dlsym(RTLD_NEXT, "epoll_ctl"));

	real_close = reinterpret_cast<Close>(::dlsym(RTLD_NEXT, "close"));
	real_write = reinterpret_cast<Write>(::dlsym(RTLD_NEXT, "write"));
	real_read = reinterpret_cast<Read>(::dlsym(RTLD_NEXT, "read"));
	real_pipe2 = reinterpret_cast<Pipe2>(::dlsym(RTLD_NEXT, "pipe2"));
	real_sendmsg = reinterpret_cast<SendMsg>(::dlsym(RTLD_NEXT, "sendmsg"));
	real_recvmsg = reinterpret_cast<RecvMsg>(::dlsym(RTLD_NEXT, "recvmsg"));
	real_eventfd = reinterpret_cast<EventFd>(::dlsym(RTLD_NEXT, "eventfd"));
	real_eventfd_read = reinterpret_cast<EventFdRead>(::dlsym(RTLD_NEXT, "eventfd_read"));
	real_eventfd_write = reinterpret_cast<EventFdWrite>(::dlsym(RTLD_NEXT, "eventfd_write"));

	real_socket = reinterpret_cast<Socket>(::dlsym(RTLD_NEXT, "socket"));
	real_bind = reinterpret_cast<Bind>(::dlsym(RTLD_NEXT, "bind"));
	real_listen = reinterpret_cast<Listen>(::dlsym(RTLD_NEXT, "listen"));
	real_accept4 = reinterpret_cast<Accept4>(::dlsym(RTLD_NEXT, "accept4"));
	real_connect = reinterpret_cast<Connect>(::dlsym(RTLD_NEXT, "connect"));
	real_shutdown = reinterpret_cast<Shutdown>(::dlsym(RTLD_NEXT, "shutdown"));
	real_getsockopt = reinterpret_cast<GetSockOpt>(::dlsym(RTLD_NEXT, "getsockopt"));
}

void MockSysCallEnvironment::registerMock(SysCall *mock)
{
	sysMock = mock;
}

void MockSysCallEnvironment::restoreSysCall()
{
	sysMock = defaultSysMock;
}

extern "C" {
int epoll_create(int size)
{
	return MockSysCallEnvironment::sysMock->mock_epoll_create(size);
}

int epoll_create1(int flags)
{
	return MockSysCallEnvironment::sysMock->mock_epoll_create1(flags);
}

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	return MockSysCallEnvironment::sysMock->mock_epoll_wait(epfd, events, maxevents, timeout);
}

int epoll_pwait(int epfd, struct epoll_event *events, int maxevents, int timeout, const sigset_t *sigmask)
{
	return MockSysCallEnvironment::sysMock->mock_epoll_pwait(epfd, events, maxevents, timeout, sigmask);
}

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
	return MockSysCallEnvironment::sysMock->mock_epoll_ctl(epfd, op, fd, event);
}

int close(int fd)
{
	return MockSysCallEnvironment::sysMock->mock_close(fd);
}

::ssize_t write(int fd, const void *buf, ::size_t count)
{
	return MockSysCallEnvironment::sysMock->mock_write(fd, buf, count);
}

::ssize_t read(int fd, void *buf, ::size_t count)
{
	return MockSysCallEnvironment::sysMock->mock_read(fd, buf, count);
}

int pipe2(int pipefd[2], int flags)
{
	return MockSysCallEnvironment::sysMock->mock_pipe2(pipefd, flags);
}

::ssize_t sendmsg(int sockfd, const struct ::msghdr *msg, int flags)
{
	return MockSysCallEnvironment::sysMock->mock_sendmsg(sockfd, msg, flags);
}

::ssize_t recvmsg(int sockfd, struct ::msghdr *msg, int flags)
{
	return MockSysCallEnvironment::sysMock->mock_recvmsg(sockfd, msg, flags);
}

int eventfd(unsigned int initval, int flags)
{
	return MockSysCallEnvironment::sysMock->mock_eventfd(initval, flags);
}

int eventfd_read(int fd, ::eventfd_t *value)
{
	return MockSysCallEnvironment::sysMock->mock_eventfd_read(fd, value);
}

int eventfd_write(int fd, ::eventfd_t value)
{
	return MockSysCallEnvironment::sysMock->mock_eventfd_write(fd, value);
}

int socket(int domain, int type, int protocol)
{
	return MockSysCallEnvironment::sysMock->mock_socket(domain, type, protocol);
}

int bind(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen)
{
	return MockSysCallEnvironment::sysMock->mock_bind(sockfd, addr, addrlen);
}

int listen(int sockfd, int backlog)
{
	return MockSysCallEnvironment::sysMock->mock_listen(sockfd, backlog);
}

int accept4(int sockfd, struct ::sockaddr *addr, ::socklen_t *addrlen, int flags)
{
	return MockSysCallEnvironment::sysMock->mock_accept4(sockfd, addr, addrlen, flags);
}

int connect(int sockfd, const struct ::sockaddr *addr, ::socklen_t addrlen)
{
	return MockSysCallEnvironment::sysMock->mock_connect(sockfd, addr, addrlen);
}

int shutdown(int sockfd, int how)
{
	return MockSysCallEnvironment::sysMock->mock_shutdown(sockfd, how);
}

int getsockopt(int sockfd, int level, int optname, void *optval, ::socklen_t *optlen)
{
	return MockSysCallEnvironment::sysMock->mock_getsockopt(sockfd, level, optname, optval, optlen);
}
}
