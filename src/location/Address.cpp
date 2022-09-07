//
// Created by gaojian on 2020/7/5.
//

#include "location/Address.h"

namespace netpp {
Address::Address()
	: m_port{0}
{}

Address::Address(std::string ip, uint16_t port)
	: m_ipAddress{std::move(ip)}, m_port{port}
{}

bool Address::isValid() const
{
	return !m_ipAddress.empty() && m_port != 0;
}
}
