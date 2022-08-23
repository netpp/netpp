//
// Created by gaojian on 22-7-18.
//

#include "channel/TcpChannel.h"
#include "epoll/handlers/SocketConnectionHandler.h"
#include "buffer/TcpBuffer.h"
#include "buffer/ByteArray.h"
#include "eventloop/EventLoop.h"

namespace netpp {
class TcpChannelImpl : public Channel, public std::enable_shared_from_this<TcpChannelImpl> {
public:
	TcpChannelImpl(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket)
			: _loop{loop}, _tmpSocketDev{std::move(socket)}
	{}

	~TcpChannelImpl() override = default;

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

TcpChannel::TcpChannel(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket)
	: m_impl{std::make_shared<TcpChannelImpl>(loop, std::move(socket))}
{
	m_impl->init();
}

TcpChannel::~TcpChannel() = default;
}
