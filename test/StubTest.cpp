#include <gtest/gtest.h>
#include "stub/Epoll.h"
#include "stub/IO.h"
#include "stub/Socket.h"
#include "MockSysCallEnvironment.h"
#include <gmock/gmock.h>
#include "error/Exception.h"

class StubSysCall : public SysCall {
public:
	~StubSysCall() = default;

	MOCK_METHOD(int, mock_epoll_create, (int), (override));
	MOCK_METHOD(int, mock_epoll_create1, (int), (override));
	MOCK_METHOD(int, mock_epoll_wait, (int, struct epoll_event *, int, int), (override));
	MOCK_METHOD(int, mock_epoll_pwait, (int, struct epoll_event *, int, int, const sigset_t *), (override));
	MOCK_METHOD(int, mock_epoll_ctl, (int, int, int, struct epoll_event *), (override));
	
	MOCK_METHOD(int, mock_close, (int), (override));
	MOCK_METHOD(::ssize_t, mock_write, (int, const void *, ::size_t), (override));
	MOCK_METHOD(::ssize_t, mock_read, (int, void *, ::size_t), (override));
	MOCK_METHOD(int, mock_pipe2, (int[2], int), (override));
	MOCK_METHOD(::ssize_t, mock_sendmsg, (int, const struct ::msghdr *, int), (override));
	MOCK_METHOD(::ssize_t, mock_recvmsg, (int, struct ::msghdr *, int), (override));
	MOCK_METHOD(int, mock_eventfd, (unsigned int, int), (override));
	MOCK_METHOD(int, mock_eventfd_read, (int, ::eventfd_t *), (override));
	MOCK_METHOD(int, mock_eventfd_write, (int, ::eventfd_t), (override));

	MOCK_METHOD(int, mock_socket, (int, int, int), (override));
	MOCK_METHOD(int, mock_bind, (int, const struct ::sockaddr *, ::socklen_t), (override));
	MOCK_METHOD(int, mock_listen, (int, int), (override));
	MOCK_METHOD(int, mock_accept4, (int, struct ::sockaddr *, ::socklen_t *, int), (override));
	MOCK_METHOD(int, mock_connect, (int, const struct ::sockaddr *, ::socklen_t), (override));
	MOCK_METHOD(int, mock_shutdown, (int, int), (override));
	MOCK_METHOD(int, mock_getsockopt, (int, int, int, void *, ::socklen_t *), (override));
};

class StubTest : public testing::Test {
protected:
	void SetUp() override
	{
		MockSysCallEnvironment::registerMock(&mock);
	}

	void TearDown() override
	{
		MockSysCallEnvironment::restoreSysCall();
	}

	StubSysCall mock;
};

TEST_F(StubTest, EpollStubPassArgTest)
{
	EXPECT_CALL(mock, mock_epoll_create1(1)).Times(1);
	netpp::internal::stub::epoll_create1(1);
	EXPECT_CALL(mock, mock_epoll_create1(2)).Times(1);
	netpp::internal::stub::epoll_create1(2);

	EXPECT_CALL(mock, mock_epoll_ctl(1, 2, 3, nullptr)).Times(1);
	netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr);
	EXPECT_CALL(mock, mock_epoll_ctl(4, 5, 6, reinterpret_cast<::epoll_event *>(7))).Times(1);
	netpp::internal::stub::epoll_ctl(4, 5, 6, reinterpret_cast<::epoll_event *>(7));

	EXPECT_CALL(mock, mock_epoll_wait(1, nullptr, 2, 3)).Times(1);
	netpp::internal::stub::epoll_wait(1, nullptr, 2, 3);
	EXPECT_CALL(mock, mock_epoll_wait(4, reinterpret_cast<::epoll_event *>(5), 6, 7)).Times(1);
	netpp::internal::stub::epoll_wait(4, reinterpret_cast<::epoll_event *>(5), 6, 7);
}

TEST_F(StubTest, IOStubPassArgTest)
{
	EXPECT_CALL(mock, mock_close(1)).Times(1);
	netpp::internal::stub::close(1);
	EXPECT_CALL(mock, mock_close(2)).Times(1);
	netpp::internal::stub::close(2);

	EXPECT_CALL(mock, mock_write(1, nullptr, 2)).Times(1);
	netpp::internal::stub::write(1, nullptr, 2);
	EXPECT_CALL(mock, mock_write(3, reinterpret_cast<void *>(4), 5)).Times(1);
	netpp::internal::stub::write(3, reinterpret_cast<void *>(4), 5);

	EXPECT_CALL(mock, mock_read(1, nullptr, 2)).Times(1);
	netpp::internal::stub::read(1, nullptr, 2);
	EXPECT_CALL(mock, mock_read(3, reinterpret_cast<void *>(4), 5)).Times(1);
	netpp::internal::stub::read(3, reinterpret_cast<void *>(4), 5);

	EXPECT_CALL(mock, mock_pipe2(nullptr, 1)).Times(1);
	netpp::internal::stub::pipe2(nullptr, 1);
	EXPECT_CALL(mock, mock_pipe2(reinterpret_cast<int *>(4), 3)).Times(1);
	netpp::internal::stub::pipe2(reinterpret_cast<int *>(4), 3);

	EXPECT_CALL(mock, mock_sendmsg(1, reinterpret_cast<::msghdr *>(2), 3)).Times(1);
	netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3);
	EXPECT_CALL(mock, mock_sendmsg(4, reinterpret_cast<::msghdr *>(5), 6)).Times(1);
	netpp::internal::stub::sendmsg(4, reinterpret_cast<::msghdr *>(5), 6);

	EXPECT_CALL(mock, mock_recvmsg(1, reinterpret_cast<::msghdr *>(2), 3)).Times(1);
	netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3);
	EXPECT_CALL(mock, mock_recvmsg(4, reinterpret_cast<::msghdr *>(5), 6)).Times(1);
	netpp::internal::stub::recvmsg(4, reinterpret_cast<::msghdr *>(5), 6);

	EXPECT_CALL(mock, mock_eventfd(1, 2)).Times(1);
	netpp::internal::stub::eventfd(1, 2);
	EXPECT_CALL(mock, mock_eventfd(3, 4)).Times(1);
	netpp::internal::stub::eventfd(3, 4);

	EXPECT_CALL(mock, mock_eventfd_read(1, reinterpret_cast<::eventfd_t *>(2))).Times(1);
	netpp::internal::stub::eventfd_read(1, reinterpret_cast<::eventfd_t *>(2));
	EXPECT_CALL(mock, mock_eventfd_read(3, reinterpret_cast<::eventfd_t *>(4))).Times(1);
	netpp::internal::stub::eventfd_read(3, reinterpret_cast<::eventfd_t *>(4));

	EXPECT_CALL(mock, mock_eventfd_write(1, 2)).Times(1);
	netpp::internal::stub::eventfd_write(1, 2);
	EXPECT_CALL(mock, mock_eventfd_write(3, 4)).Times(1);
	netpp::internal::stub::eventfd_write(3, 4);
}

TEST_F(StubTest, SocketStubPassArgTest)
{
	EXPECT_CALL(mock, mock_socket(1, 2, 3)).Times(1);
	netpp::internal::stub::socket(1, 2, 3);
	EXPECT_CALL(mock, mock_socket(4, 5, 7)).Times(1);
	netpp::internal::stub::socket(4, 5, 7);

	EXPECT_CALL(mock, mock_bind(1, reinterpret_cast<sockaddr *>(2), 3)).Times(1);
	netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3);
	EXPECT_CALL(mock, mock_bind(4, reinterpret_cast<sockaddr *>(5), 6)).Times(1);
	netpp::internal::stub::bind(4, reinterpret_cast<sockaddr *>(5), 6);

	EXPECT_CALL(mock, mock_listen(1, 2)).Times(1);
	netpp::internal::stub::listen(1, 2);
	EXPECT_CALL(mock, mock_listen(3, 4)).Times(1);
	netpp::internal::stub::listen(3, 4);

	EXPECT_CALL(mock, mock_accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4)).Times(1);
	netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4);
	EXPECT_CALL(mock, mock_accept4(5, reinterpret_cast<sockaddr *>(6), reinterpret_cast<socklen_t *>(7), 8)).Times(1);
	netpp::internal::stub::accept4(5, reinterpret_cast<sockaddr *>(6), reinterpret_cast<socklen_t *>(7), 8);
	
	EXPECT_CALL(mock, mock_connect(1, reinterpret_cast<sockaddr *>(2), 3)).Times(1);
	netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3);
	EXPECT_CALL(mock, mock_connect(4, reinterpret_cast<sockaddr *>(5), 6)).Times(1);
	netpp::internal::stub::connect(4, reinterpret_cast<sockaddr *>(5), 6);

	EXPECT_CALL(mock, mock_shutdown(1, 2)).Times(1);
	netpp::internal::stub::shutdown(1, 2);
	EXPECT_CALL(mock, mock_shutdown(3, 4)).Times(1);
	netpp::internal::stub::shutdown(3, 4);

	EXPECT_CALL(mock, mock_getsockopt(1, 2, 3, reinterpret_cast<void *>(4), reinterpret_cast<socklen_t *>(5))).Times(1);
	netpp::internal::stub::getsockopt(1, 2, 3, reinterpret_cast<void *>(4), reinterpret_cast<socklen_t *>(5));
	EXPECT_CALL(mock, mock_getsockopt(6, 7, 8, reinterpret_cast<void *>(9), reinterpret_cast<socklen_t *>(10))).Times(1);
	netpp::internal::stub::getsockopt(6, 7, 8, reinterpret_cast<void *>(9), reinterpret_cast<socklen_t *>(10));
}

TEST_F(StubTest, EpollCreate1ErrnoTest)
{
	EXPECT_CALL(mock, mock_epoll_create1)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOMEM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(1)));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_create1(1));
	EXPECT_THROW(netpp::internal::stub::epoll_create1(1), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::epoll_create1(1), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::epoll_create1(1), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::epoll_create1(1));
}

TEST_F(StubTest, EpollCtlErrnoTest)
{
	EXPECT_CALL(mock, mock_epoll_ctl)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EEXIST), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ELOOP), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOENT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOMEM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOSPC), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EPERM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr));
	EXPECT_THROW(netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_ctl(1, 2, 3, nullptr));
}

TEST_F(StubTest, EpollWaitErrnoTest)
{
	EXPECT_CALL(mock, mock_epoll_wait)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EFAULT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_wait(1, nullptr, 2, 3));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_wait(1, nullptr, 2, 3));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_wait(1, nullptr, 2, 3));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_wait(1, nullptr, 2, 3));

	EXPECT_CALL(mock, mock_epoll_wait)
		.Times(3)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)));
	EXPECT_NO_THROW(netpp::internal::stub::epoll_wait(1, nullptr, 2, 3));
}

TEST_F(StubTest, CloseErrnoTest)
{
	EXPECT_CALL(mock, mock_close)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EIO), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOSPC), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EDQUOT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::close(1));
	EXPECT_NO_THROW(netpp::internal::stub::close(1));
	EXPECT_NO_THROW(netpp::internal::stub::close(1));
	EXPECT_NO_THROW(netpp::internal::stub::close(1));
	EXPECT_NO_THROW(netpp::internal::stub::close(1));

	EXPECT_CALL(mock, mock_close)
		.Times(1)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)));
	EXPECT_NO_THROW(netpp::internal::stub::close(1));
}

TEST_F(StubTest, WriteErrnoTest)
{
	EXPECT_CALL(mock, mock_write)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EAGAIN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EWOULDBLOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EDESTADDRREQ), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EDQUOT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EFAULT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EFBIG), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EIO), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOSPC), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EPERM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EPIPE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));

	EXPECT_CALL(mock, mock_write)
		.Times(3)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)));
	EXPECT_NO_THROW(netpp::internal::stub::write(1, nullptr, 2));
}

TEST_F(StubTest, ReadErrnoTest)
{
	EXPECT_CALL(mock, mock_read)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EAGAIN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EWOULDBLOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EFAULT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EIO), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EISDIR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::read(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::read(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::read(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::read(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::read(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::read(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::read(1, nullptr, 2));
	EXPECT_NO_THROW(netpp::internal::stub::read(1, nullptr, 2));
	
	EXPECT_CALL(mock, mock_read)
		.Times(3)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)));
	EXPECT_NO_THROW(netpp::internal::stub::read(1, nullptr, 2));
}

TEST_F(StubTest, Pipe2ErrnoTest)
{
	EXPECT_CALL(mock, mock_pipe2)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EFAULT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::pipe2(nullptr, 1));
	EXPECT_NO_THROW(netpp::internal::stub::pipe2(nullptr, 1));
	EXPECT_THROW(netpp::internal::stub::pipe2(nullptr, 1), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::pipe2(nullptr, 1), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::pipe2(nullptr, 1));
}

TEST_F(StubTest, SendmsgErrnoTest)
{
	EXPECT_CALL(mock, mock_sendmsg)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EAGAIN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EWOULDBLOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EAFNOSUPPORT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ECONNRESET), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMSGSIZE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTCONN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTSOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EOPNOTSUPP), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EPIPE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ELOOP), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENAMETOOLONG), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOENT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTDIR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EACCES), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EDESTADDRREQ), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EHOSTUNREACH), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EIO), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EISCONN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENETDOWN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENETUNREACH), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOBUFS), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOMEM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));

	EXPECT_CALL(mock, mock_sendmsg)
		.Times(3)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)));
	EXPECT_NO_THROW(netpp::internal::stub::sendmsg(1, reinterpret_cast<::msghdr *>(2), 3));
}

TEST_F(StubTest, RecvmsgErrnoTest)
{
	EXPECT_CALL(mock, mock_recvmsg)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EAGAIN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EWOULDBLOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ECONNRESET), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMSGSIZE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTCONN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTSOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EOPNOTSUPP), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ETIMEDOUT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EIO), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOBUFS), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOMEM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
	
	EXPECT_CALL(mock, mock_recvmsg)
		.Times(3)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)));
	EXPECT_NO_THROW(netpp::internal::stub::recvmsg(1, reinterpret_cast<::msghdr *>(2), 3));
}

TEST_F(StubTest, EventfdErrnoTest)
{
	EXPECT_CALL(mock, mock_eventfd)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENODEV), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOMEM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::eventfd(1, 2));
	EXPECT_THROW(netpp::internal::stub::eventfd(1, 2), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::eventfd(1, 2), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::eventfd(1, 2), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::eventfd(1, 2), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::eventfd(1, 2));
}

TEST_F(StubTest, SocketErrnoTest)
{
	EXPECT_CALL(mock, mock_socket)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EACCES), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EAFNOSUPPORT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOBUFS), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOMEM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EPROTONOSUPPORT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_THROW(netpp::internal::stub::socket(1, 2, 3), netpp::error::SocketException);
	EXPECT_THROW(netpp::internal::stub::socket(1, 2, 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::socket(1, 2, 3));
	EXPECT_THROW(netpp::internal::stub::socket(1, 2, 3), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::socket(1, 2, 3), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::socket(1, 2, 3), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::socket(1, 2, 3), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::socket(1, 2, 3));
	EXPECT_NO_THROW(netpp::internal::stub::socket(1, 2, 3));
}

TEST_F(StubTest, BindErrnoTest)
{
	EXPECT_CALL(mock, mock_bind)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EACCES), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EADDRINUSE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTSOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EADDRNOTAVAIL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EFAULT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ELOOP), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENAMETOOLONG), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOENT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOMEM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTDIR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EROFS), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::bind(1, reinterpret_cast<sockaddr *>(2), 3));
}

TEST_F(StubTest, ListenErrnoTest)
{
	EXPECT_CALL(mock, mock_listen)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EDESTADDRREQ), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTSOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EOPNOTSUPP), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EACCES), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOBUFS), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_THROW(netpp::internal::stub::listen(1, 2), netpp::error::SocketException);
	EXPECT_THROW(netpp::internal::stub::listen(1, 2), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::listen(1, 2));
	EXPECT_THROW(netpp::internal::stub::listen(1, 2), netpp::error::SocketException);
	EXPECT_THROW(netpp::internal::stub::listen(1, 2), netpp::error::SocketException);
	EXPECT_THROW(netpp::internal::stub::listen(1, 2), netpp::error::SocketException);
	EXPECT_THROW(netpp::internal::stub::listen(1, 2), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::listen(1, 2));
}

TEST_F(StubTest, AcceptErrnoTest)
{
	EXPECT_CALL(mock, mock_accept4)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EAGAIN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EWOULDBLOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ECONNABORTED), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EFAULT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EMFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENFILE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOBUFS), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOMEM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTSOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EOPNOTSUPP), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EPROTO), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EPERM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
	EXPECT_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
	EXPECT_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4), netpp::error::ResourceLimitException);
	EXPECT_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
	EXPECT_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4), netpp::error::ResourceLimitException);
	EXPECT_NO_THROW(netpp::internal::stub::accept4(1, reinterpret_cast<sockaddr *>(2), reinterpret_cast<socklen_t *>(3), 4));
}

TEST_F(StubTest, ConnectErrnoTest)
{
	EXPECT_CALL(mock, mock_connect)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EACCES), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EPERM), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EADDRINUSE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EADDRNOTAVAIL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EAFNOSUPPORT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EAGAIN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EALREADY), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ECONNREFUSED), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EFAULT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINPROGRESS), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINTR), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EISCONN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENETUNREACH), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTSOCK), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EPROTOTYPE), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ETIMEDOUT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, 0), testing::Return(0)));
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3), netpp::error::SocketException);
	EXPECT_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
	EXPECT_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3), netpp::error::SocketException);
	EXPECT_NO_THROW(netpp::internal::stub::connect(1, reinterpret_cast<sockaddr *>(2), 3));
}

TEST_F(StubTest, ShutdownErrnoTest)
{
	EXPECT_CALL(mock, mock_shutdown)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTCONN), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTSOCK), testing::Return(-1)));
	EXPECT_NO_THROW(netpp::internal::stub::shutdown(3, 4));
	EXPECT_NO_THROW(netpp::internal::stub::shutdown(3, 4));
	EXPECT_NO_THROW(netpp::internal::stub::shutdown(3, 4));
	EXPECT_NO_THROW(netpp::internal::stub::shutdown(3, 4));
}

TEST_F(StubTest, GetSockOptErrnoTest)
{
	EXPECT_CALL(mock, mock_getsockopt)
		.WillOnce(testing::DoAll(testing::Assign(&errno, EBADF), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EFAULT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, EINVAL), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOPROTOOPT), testing::Return(-1)))
		.WillOnce(testing::DoAll(testing::Assign(&errno, ENOTSOCK), testing::Return(-1)));
	EXPECT_NO_THROW(netpp::internal::stub::getsockopt(1, 2, 3, reinterpret_cast<void *>(4), reinterpret_cast<socklen_t *>(5)));
	EXPECT_NO_THROW(netpp::internal::stub::getsockopt(1, 2, 3, reinterpret_cast<void *>(4), reinterpret_cast<socklen_t *>(5)));
	EXPECT_NO_THROW(netpp::internal::stub::getsockopt(1, 2, 3, reinterpret_cast<void *>(4), reinterpret_cast<socklen_t *>(5)));
	EXPECT_NO_THROW(netpp::internal::stub::getsockopt(1, 2, 3, reinterpret_cast<void *>(4), reinterpret_cast<socklen_t *>(5)));
	EXPECT_NO_THROW(netpp::internal::stub::getsockopt(1, 2, 3, reinterpret_cast<void *>(4), reinterpret_cast<socklen_t *>(5)));
}
