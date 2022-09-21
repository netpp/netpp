//
// Created by gaojian on 22-9-4.
//

#ifndef NETPP_DATAGRAM_H
#define NETPP_DATAGRAM_H

#include "ByteArray.h"

struct sockaddr_in;

namespace netpp {
class Address;
class Datagram : public ByteArray {
	friend ::sockaddr_in *datagramDestinationExtractor(Datagram *);
public:
	Datagram();
	~Datagram() override;
	Datagram(Datagram &other, LengthType size, bool move);

	Address destination() const;
	void setDestination(const Address &address);

private:
	::sockaddr_in *m_address;
};
} // netpp

#endif //NETPP_DATAGRAM_H
