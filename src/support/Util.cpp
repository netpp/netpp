//
// Created by gaojian on 22-6-16.
//

#include "support/Util.h"
#include "location/Address.h"
#include <cstring>
extern "C" {
#include <netinet/in.h>
#include <arpa/inet.h>
}

namespace netpp {
::sockaddr_in toSockAddress(const Address &address)
{
	::sockaddr_in sockAddress;
	std::memset(&sockAddress, 0, sizeof(::sockaddr_in));
	sockAddress.sin_family = AF_INET;
	std::string ip = address.ip();
	if (ip == "0.0.0.0")
		sockAddress.sin_addr.s_addr = ::htons(INADDR_ANY);
	else
		sockAddress.sin_addr.s_addr = ::inet_addr(ip.c_str());
	sockAddress.sin_port = ::htons(address.port());

	return sockAddress;
}

Address toAddress(const ::sockaddr_in &address)
{
//	char buffer[INET_ADDRSTRLEN];
	// inet_ntop(AF_INET6, &m_addr->sin_addr, addr, INET6_ADDRSTRLEN);
//	::inet_ntop(AF_INET, &address.sin_addr, buffer, INET_ADDRSTRLEN);
	std::string ip(::inet_ntoa(address.sin_addr));
	uint16_t port = ::ntohs(address.sin_port);
	return Address(ip, port);
}
}
