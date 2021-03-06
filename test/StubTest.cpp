#include <gtest/gtest.h>
#include "stub/Epoll.h"
#include "stub/IO.h"
#include "stub/Socket.h"
#include "MockSysCallEnvironment.h"
#include <gmock/gmock.h>

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

TEST_F(StubTest, EpollStubTest)
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

TEST_F(StubTest, IOStubTest)
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
}

TEST_F(StubTest, SocketStubTest)
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
