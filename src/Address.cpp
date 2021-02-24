//
// Created by gaojian on 2020/7/5.
//

#include "Address.h"
#include <cstring>

extern "C" {
#include <netinet/in.h>
#include <arpa/inet.h>
}

using std::string;

namespace netpp {
	
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
Address::Address(const std::string &ip, unsigned port)
	: m_addr{std::make_shared<::sockaddr_in>()}
{
	std::memset(m_addr.get(), 0, sizeof(::sockaddr_in));
	m_addr->sin_family = AF_INET;
	if (ip == "0.0.0.0")
		m_addr->sin_addr.s_addr = ::htons(INADDR_ANY);
	else
		m_addr->sin_addr.s_addr = ::inet_addr(ip.c_str());
	m_addr->sin_port = ::htons(port);
}
#pragma GCC diagnostic pop

Address::Address(std::shared_ptr<::sockaddr_in> addr)
	: m_addr{addr}
{}

std::string Address::ip() const
{
	char addr[INET_ADDRSTRLEN];
	// inet_ntop(AF_INET6, &m_addr->sin_addr, addr, INET6_ADDRSTRLEN);
	::inet_ntop(AF_INET, &m_addr->sin_addr, addr, INET_ADDRSTRLEN);
	return string(::inet_ntoa(m_addr->sin_addr));
}

unsigned int Address::port() const
{
	return ::ntohs(m_addr->sin_port);
}
}
