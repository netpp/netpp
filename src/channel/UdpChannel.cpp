//
// Created by gaojian on 22-8-23.
//

#include "channel/UdpChannel.h"
#include "eventloop/EventLoop.h"
#include "iodevice/UdpSocket.h"
#include "buffer/TcpBuffer.h"
#include "epoll/handlers/SocketConnectionHandler.h"

namespace netpp {
class UdpChannelImpl : public Channel, public std::enable_shared_from_this<UdpChannelImpl> {
public:
	UdpChannelImpl(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket)
			: _loop{loop}, _tmpSocketDev{std::move(socket)}
	{}
	~UdpChannelImpl() override = default;

	void init()
	{
		auto buffer = std::make_shared<TcpBuffer>();
		_buffer = buffer;
		auto connection = std::make_shared<SocketConnectionHandler>(_loop, std::move(_tmpSocketDev),
																	shared_from_this(), buffer);
		_connection = connection;
		_loop->addEventHandlerToLoop(connection);
	}

private:
	EventLoop *_loop;
	std::unique_ptr<SocketDevice> &&_tmpSocketDev;
};

UdpChannel::UdpChannel(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket)
		: m_impl{std::make_shared<UdpChannelImpl>(loop, std::move(socket))}
{
	m_impl->init();
}

UdpChannel::~UdpChannel() = default;
}
