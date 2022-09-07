//
// Created by gaojian on 22-8-23.
//

#include "channel/UdpChannel.h"
#include "eventloop/EventLoop.h"
#include "iodevice/UdpSocket.h"
#include "epoll/handlers/SocketConnectionHandler.h"
#include "buffer/TransferBuffer.h"
#include "buffer/Datagram.h"
#include "buffer/BufferIOConversion.h"
#include <queue>

namespace netpp {
class UdpBuffer : public TransferBuffer {
public:
	UdpBuffer() = default;
	~UdpBuffer() override = default;

	void write(const netpp::ByteArray &buffer) override
	{
		m_sendBuffers.emplace_back(std::make_shared<Datagram>(dynamic_cast<const Datagram &>(buffer)));
	}

	ByteArray peek(ByteArray::LengthType size) override
	{
		if (!m_receiveBuffers.empty())
		{
			return ByteArray(*m_receiveBuffers.front(), size, false);
		}
		return ByteArray();
	}

	ByteArray read(ByteArray::LengthType size) override
	{
		if (!m_receiveBuffers.empty())
		{
			auto front = m_receiveBuffers.front();
			if (front->readableBytes() <= size)
			{
				ByteArray data(*front);
				m_receiveBuffers.pop_front();
				return data;
			}
			else
				return ByteArray(*front, size, true);
		}
		return ByteArray();
	}

	std::unique_ptr<ByteArrayGather> sendBufferForIO() override
	{
		auto conv = std::make_unique<DatagramReaderWithLock>(m_sendBuffers.front());
		m_sendBuffers.pop_front();
		return conv;
	}

	std::unique_ptr<ByteArrayGather> receiveBufferForIO() override
	{
		auto buffer = std::make_shared<Datagram>();
		m_receiveBuffers.emplace_back(buffer);
		return std::make_unique<ByteArrayWriterWithLock>(buffer);
	}

	ByteArray::LengthType bytesReceived() const override
	{
		ByteArray::LengthType size = 0;
		for (auto &s : m_receiveBuffers)
			size += s->readableBytes();
		return size;
	}

	ByteArray::LengthType bytesCanBeSend() const override
	{
		ByteArray::LengthType size = 0;
		for (auto &s : m_sendBuffers)
			size += s->readableBytes();
		return size;
	}

private:
	std::deque<std::shared_ptr<Datagram>> m_sendBuffers;
	std::deque<std::shared_ptr<Datagram>> m_receiveBuffers;
};

class UdpChannelImpl : public Channel, public std::enable_shared_from_this<UdpChannelImpl> {
public:
	UdpChannelImpl(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket)
			: _loop{loop}, _tmpSocketDev{std::move(socket)}
	{}
	~UdpChannelImpl() override = default;

	void init()
	{
		auto buffer = std::make_shared<UdpBuffer>();
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
