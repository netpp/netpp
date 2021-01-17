//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_ADDRESS_H
#define NETPP_ADDRESS_H

#include <string>
#include <memory>

struct sockaddr_in;

namespace netpp {
// TODO: suppor ipv6
class Address {
public:
	explicit Address(const std::string &ip = "0.0.0.0", unsigned port = 11111);
	explicit Address(std::shared_ptr<::sockaddr_in> addr);

	std::string ip() const;
	unsigned port() const;

	// bool ipv6();

	inline ::sockaddr_in *sockAddrIn() { return m_addr.get(); }

private:
	std::shared_ptr<::sockaddr_in> m_addr;
};
}

#endif //NETPP_ADDRESS_H
