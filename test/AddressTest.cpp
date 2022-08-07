#include <gtest/gtest.h>
#include "location/Address.h"
#include "support/Util.h"
extern "C" {
#include <netinet/in.h>
#include <arpa/inet.h>
}

class AddressTest : public testing::Test {
protected:
	void SetUp() override {}
	void TearDown() override {}
};

TEST_F(AddressTest, IpV4AddressTest)
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

TEST_F(AddressTest, IpV6AddressTest)
{}

TEST_F(AddressTest, IpV4AddressConvertionTest)
{
	netpp::Address addr1("192.168.0.1", 10);
	::sockaddr_in sockaddr1 = netpp::toSockAddress(addr1);
	EXPECT_EQ(sockaddr1.sin_family, AF_INET);
	EXPECT_EQ(sockaddr1.sin_addr.s_addr, ::inet_addr("192.168.0.1"));
	EXPECT_EQ(sockaddr1.sin_port, ::htons(10));

	netpp::Address addr2("0.0.0.0", 20);
	::sockaddr_in sockaddr2 = netpp::toSockAddress(addr1);
	EXPECT_EQ(sockaddr2.sin_family, AF_INET);
	EXPECT_EQ(sockaddr2.sin_addr.s_addr, ::htons(INADDR_ANY));
	EXPECT_EQ(sockaddr2.sin_port, ::htons(20));

	auto addr3 = netpp::toAddress(sockaddr1);
	EXPECT_EQ(std::string("192.168.0.1"), addr3.ip());
	EXPECT_EQ(10, addr3.port());

	auto addr4 = netpp::toAddress(sockaddr2);
	EXPECT_EQ(std::string("0.0.0.0"), addr4.ip());
	EXPECT_EQ(20, addr4.port());
}