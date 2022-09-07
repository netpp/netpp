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
	Datagram readDatagram();
	void connect(const Address &address);

private:
	Address m_defaultDestination;
	std::shared_ptr<UdpChannelImpl> m_impl;
};
}

#endif //NETPP_UDPCHANNEL_H
