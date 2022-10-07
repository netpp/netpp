//
// Created by 17271 on 2022/6/5.
//

#ifndef NETPP_SOCKETDEVICE_H
#define NETPP_SOCKETDEVICE_H

#include "IODevice.h"

struct msghdr;

namespace netpp {
enum class SocketError;
class Address;

enum class SocketState {
	E_TimeOut,
	E_ConnectRefused,
	E_InProgress,
	E_Ok,
	E_Unknown
};

class SocketDevice : public IODevice {
public:
	~SocketDevice() noexcept override = default;

	void read(std::shared_ptr<BufferGather> buffer) override;
	std::size_t write(std::shared_ptr<BufferGather> buffer) override;

	virtual Address address() = 0;

	virtual void bind(const Address &address) = 0;
	virtual void listen() = 0;
	virtual SocketDevice *accept() = 0;
	virtual SocketState state() = 0;
	virtual void shutdownWrite() = 0;
	virtual bool connect(const Address &address) = 0;

protected:
	virtual std::size_t realSend(::msghdr *msg) = 0;
	virtual std::size_t realRecv(::msghdr *msg) = 0;
};
}

#endif //NETPP_SOCKETDEVICE_H
