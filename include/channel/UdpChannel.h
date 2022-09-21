//
// Created by gaojian on 22-8-23.
//

#ifndef NETPP_UDPCHANNEL_H
#define NETPP_UDPCHANNEL_H

#include "Channel.h"
#include "location/Address.h"

namespace netpp {
class UdpChannelImpl;
class EventLoop;
class SocketDevice;
class Datagram;
class UdpChannel : public Channel {
public:
	UdpChannel(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket);
	~UdpChannel() override;

	void sendDatagram(const Datagram &data);
	Datagram peekDatagram(ByteArray::LengthType size);
	Datagram readDatagram(ByteArray::LengthType size);

private:
	std::shared_ptr<UdpChannelImpl> m_impl;
};
}

#endif //NETPP_UDPCHANNEL_H
