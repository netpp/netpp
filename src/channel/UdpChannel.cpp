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
			return ByteArray{*m_receiveBuffers.front(), size, false};
		}
		return {};
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
				return ByteArray{*front, size, true};
		}
		return {};
	}

	Datagram peekDatagram(ByteArray::LengthType size)
	{
		if (!m_receiveBuffers.empty())
		{
			return Datagram{*m_receiveBuffers.front(), size, false};
		}
		return {};
	}

	Datagram readDatagram(ByteArray::LengthType size)
	{
		if (!m_receiveBuffers.empty())
		{
			auto front = m_receiveBuffers.front();
			if (front->readableBytes() <= size)
			{
				Datagram data(*front);
				m_receiveBuffers.pop_front();
				return data;
			}
			else
				return Datagram{*front, size, true};
		}
		return {};
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
		return std::make_unique<DatagramWriterWithLock>(buffer);
	}

	[[nodiscard]] ByteArray::LengthType bytesReceived() const override
	{
		ByteArray::LengthType size = 0;
		for (auto &s : m_receiveBuffers)
			size += s->readableBytes();
		return size;
	}

	[[nodiscard]] ByteArray::LengthType bytesCanBeSend() const override
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
	explicit UdpChannelImpl(EventLoop *loop)
			: _loop{loop}
	{}
	~UdpChannelImpl() override = default;

	void init(std::unique_ptr<SocketDevice> &&socket)
	{
		auto buffer = std::make_shared<UdpBuffer>();
		_buffer = buffer;
		auto connection = std::make_shared<SocketConnectionHandler>(_loop, std::move(socket),
																	shared_from_this(), buffer);
		_connection = connection;
		_loop->addEventHandlerToLoop(connection);
	}

	void sendDatagram(const Datagram &data)
	{
		auto buffer = _buffer.lock();
		if (buffer)
			buffer->write(data);
	}

	Datagram peekDatagram(ByteArray::LengthType size)
	{
		auto buffer = std::dynamic_pointer_cast<UdpBuffer>(_buffer.lock());
		if (buffer)
			return buffer->peekDatagram(size);
		return {};
	}

	Datagram readDatagram(ByteArray::LengthType size)
	{
		auto buffer = std::dynamic_pointer_cast<UdpBuffer>(_buffer.lock());
		if (buffer)
			return buffer->readDatagram(size);
		return {};
	}

private:
	EventLoop *_loop;
};

UdpChannel::UdpChannel(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket)
		: m_impl{std::make_shared<UdpChannelImpl>(loop)}
{
	m_impl->init(std::move(socket));
}

UdpChannel::~UdpChannel() = default;

void UdpChannel::sendDatagram(const Datagram &data)
{
	m_impl->sendDatagram(data);
}

Datagram UdpChannel::peekDatagram(ByteArray::LengthType size)
{
	return m_impl->peekDatagram(size);
}

Datagram UdpChannel::readDatagram(ByteArray::LengthType size)
{
	return m_impl->readDatagram(size);
}
}
