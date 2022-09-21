//
// Created by gaojian on 22-9-4.
//

#include "buffer/Datagram.h"
#include "support/Util.h"
#include "location/Address.h"
extern "C" {
#include <arpa/inet.h>
}

namespace netpp {
Datagram::Datagram()
	: m_address{nullptr}
{
}

Datagram::~Datagram()
{
	delete m_address;
}

Datagram::Datagram(Datagram &other, LengthType size, bool move)
	: ByteArray(other, size, move), m_address{nullptr}
{}

Address Datagram::destination() const
{
	if (m_address)
		return toAddress(m_address);
	return {};
}

void Datagram::setDestination(const Address &address)
{
	if (!m_address)
		m_address = new ::sockaddr_in;
	toSockAddress(address, m_address);
}
} // netpp
