//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_ADDRESS_H
#define NETPP_ADDRESS_H

#include <string>

extern "C" {
#include <netinet/in.h>
}

namespace netpp {
// TODO: suppor ipv6
class Address {
public:
	explicit Address(const std::string &ip = "0.0.0.0", unsigned port = 11111);
	explicit Address(sockaddr_in addr);

	std::string ip() const;
	unsigned port() const;

	// bool ipv6();

	inline const sockaddr_in *sockAddrIn() const { return &m_addr; }

private:
	sockaddr_in m_addr;
};
}

#endif //NETPP_ADDRESS_H
