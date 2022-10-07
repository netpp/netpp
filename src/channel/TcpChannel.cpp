//
// Created by gaojian on 22-7-18.
//

#include "channel/TcpChannel.h"
#include "epoll/handlers/SocketConnectionHandler.h"
#include "buffer/ByteArray.h"
#include "buffer/Buffer.h"
#include "eventloop/EventLoop.h"
#include "buffer/BufferGather.h"

namespace netpp {
class TcpChannelImpl : public Channel, public std::enable_shared_from_this<TcpChannelImpl> {
public:
	explicit TcpChannelImpl(EventLoop *loop)
		: _loop{loop}
	{}

	~TcpChannelImpl() override = default;

	void init(std::unique_ptr<SocketDevice> &&socket)
	{
		m_sendBufferGather = std::make_shared<SequentialBufferReadGather>();
		m_receiveBufferGather = std::make_shared<BufferWriteGather>();
		auto connection = std::make_shared<SocketConnectionHandler>(_loop, std::move(socket),
																	shared_from_this(),
																	m_receiveBufferGather, m_sendBufferGather);
		_connection = connection;
		_loop->addEventHandlerToLoop(connection);
	}

	void send(const ByteArray &data) override
	{
		auto sendBuffer = std::dynamic_pointer_cast<MultipleBufferNodesGather>(m_sendBufferGather);
		sendBuffer->addBufferNode(extractBuffer(&data));
		auto connection = _connection.lock();
		if (connection)
			connection->sendInLoop();
	}

	BufferLength readableBytes() const override
	{
		return m_receiveBufferGather->availableBytes();
	}

	ByteArrayPeeker peek() override
	{
		return ByteArrayPeeker(m_receiveBufferGather->getBuffer());
	}

	ByteArray read() override
	{
		return ByteArray(m_receiveBufferGather->getBuffer());
	}

private:
	EventLoop *_loop;
	std::shared_ptr<BufferGather> m_sendBufferGather;
	std::shared_ptr<BufferGather> m_receiveBufferGather;
};

TcpChannel::TcpChannel(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket)
	: m_impl{std::make_shared<TcpChannelImpl>(loop)}
{
	m_impl->init(std::move(socket));
}

TcpChannel::~TcpChannel() = default;

void TcpChannel::send(const ByteArray &data)
{
	m_impl->send(data);
}

BufferLength TcpChannel::readableBytes() const
{
	return m_impl->readableBytes();
}

ByteArrayPeeker TcpChannel::peek()
{
	return m_impl->peek();
}

ByteArray TcpChannel::read()
{
	return m_impl->read();
}
}
