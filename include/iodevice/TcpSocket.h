//
// Created by 17271 on 2022/6/5.
//

#ifndef NETPP_TCPSOCKET_H
#define NETPP_TCPSOCKET_H

#include "SocketDevice.h"
#include "location/Address.h"

namespace netpp {
class TcpSocket : public SocketDevice {
public:
	TcpSocket();
	TcpSocket(int sockFd, const Address &address);
	~TcpSocket() noexcept override;

	/**
	 * @brief Create a tcp socket, file descriptor
	 */
	void open() override;
	void close() override;
	int fileDescriptor() override { return m_tcpFd; }

	Address address() override { return m_peerAddress; }
	void bind(const Address &address) override;
	void listen() override;
	SocketDevice *accept() override;
	SocketState state() override;
	void shutdownWrite() override;
	bool connect(const Address &address) override;

private:
	std::size_t realRecv(::msghdr *msg) override;
	std::size_t realSend(::msghdr *msg) override;

private:
	void realClose();

	Address m_peerAddress;
	int m_tcpFd;
};
}

#endif //NETPP_TCPSOCKET_H
