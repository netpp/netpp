//
// Created by gaojian on 2020/7/5.
//

#include "location/Address.h"

namespace netpp {
Address::Address(std::string ip, uint16_t port)
	: m_ipAddress{std::move(ip)}, m_port{port}
{}
}
