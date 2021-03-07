#include <gtest/gtest.h>
#include "Address.h"
#include "MockSysCallEnvironment.h"
extern "C" {
#include <netinet/in.h>
#include <arpa/inet.h>
}

// class MockSocket : public SysCall {
// public:
// 	~MockSocket() = default;

// 	MOCK_METHOD(int, mock_socket, (int, int, int), (override));
// 	MOCK_METHOD(int, mock_bind, (int, const struct ::sockaddr *, ::socklen_t), (override));
// 	MOCK_METHOD(int, mock_listen, (int, int), (override));
// 	MOCK_METHOD(int, mock_accept4, (int, struct ::sockaddr *, ::socklen_t *, int), (override));
// 	MOCK_METHOD(int, mock_connect, (int, const struct ::sockaddr *, ::socklen_t), (override));
// 	MOCK_METHOD(int, mock_shutdown, (int, int), (override));
// 	MOCK_METHOD(int, mock_getsockopt, (int, int, int, void *, ::socklen_t *), (override));
// };

class SocketAndAddressTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(SocketAndAddressTest, AddressInfoIpV4Test)
{
	netpp::Address addr;
	EXPECT_EQ(std::string("0.0.0.0"), addr.ip());
	EXPECT_EQ(11111, addr.port());

	netpp::Address addr1("192.168.0.1", 10);
	EXPECT_EQ(std::string("192.168.0.1"), addr1.ip());
	EXPECT_EQ(10, addr1.port());

	std::shared_ptr<::sockaddr_in> sockaddr{std::make_shared<::sockaddr_in>()};
	sockaddr->sin_family = AF_INET;
	sockaddr->sin_addr.s_addr = ::inet_addr("192.168.0.2");
	sockaddr->sin_port = ::htons(11);
	netpp::Address addr2(sockaddr);
	EXPECT_EQ(std::string("192.168.0.2"), addr2.ip());
	EXPECT_EQ(11, addr2.port());
}

TEST_F(SocketAndAddressTest, AddressInfoIpV6Test)
{}

TEST_F(SocketAndAddressTest, CreateSocketTest)
{
	// netpp::internal::socket::Socket(netpp::Address());
}
