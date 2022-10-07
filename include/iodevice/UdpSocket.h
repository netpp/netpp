//
// Created by 17271 on 2022/6/5.
//

#ifndef NETPP_UDPSOCKET_H
#define NETPP_UDPSOCKET_H

#include "SocketDevice.h"
#include "location/Address.h"

namespace netpp {
class UdpSocket : public SocketDevice {
public:
	UdpSocket();
	UdpSocket(int sockFd, const Address &address);
	~UdpSocket() noexcept override;

	Address receive(std::unique_ptr<BufferGather> &&bufferConverter);
	std::size_t send(std::unique_ptr<BufferGather> &&bufferConverter, const Address &address);

	/**
	 * @brief Create a tcp socket, file descriptor
	 */
	void open() override;
	void close() override;
	int fileDescriptor() override { return m_udpFd; }

	Address address() override { return m_peerAddress; }
	void bind(const Address &address) override;
	SocketState state() override;
	void shutdownWrite() override;
	bool connect(const Address &address) override;

private:
	std::size_t realRecv(::msghdr *msg) override;
	std::size_t realSend(::msghdr *msg) override;

private:
	void realClose();

	std::atomic_flag m_connectFlag;
	Address m_peerAddress;
	int m_udpFd;
};
}

#endif //NETPP_UDPSOCKET_H
