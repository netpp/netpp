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

SysCall *MockSysCallEnvironment::defaultSysMock;
SysCall *MockSysCallEnvironment::sysMock;

EpollCreate MockSysCallEnvironment::real_epoll_create;
EpollCreate MockSysCallEnvironment::real_epoll_create1;
EpollWait MockSysCallEnvironment::real_epoll_wait;
EpollPWait MockSysCallEnvironment::real_epoll_pwait;
EpollCtl MockSysCallEnvironment::real_epoll_ctl;
Close MockSysCallEnvironment::real_close;

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
}
