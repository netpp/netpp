//
// Created by gaojian on 22-7-18.
//

#include "channel/TcpChannel.h"
#include "epoll/handlers/SocketConnectionHandler.h"
#include "buffer/ByteArray.h"
#include "eventloop/EventLoop.h"
#include "buffer/TransferBuffer.h"
#include "buffer/BufferIOConversion.h"

namespace netpp {
/**
 * @brief The normal tcp connection buffer
 */
class TcpBuffer : public TransferBuffer {
public:
	TcpBuffer()
			: m_receiveArray{std::make_shared<netpp::ByteArray>()}
	{}
	~TcpBuffer() override = default;

	void write(const ByteArray &buffer) override
	{
		m_sendBuffers.emplace_back(std::make_shared<ByteArray>(buffer));
	}

	ByteArray peek(ByteArray::LengthType size) override
	{
		return ByteArray(*m_receiveArray, size, false);
	}

	ByteArray read(ByteArray::LengthType size) override
	{
		return ByteArray(*m_receiveArray, size, true);
	}

	/**
	* @brief Read from buffer, and send to peer
	* @return The ByteArray to iovec convertor
	*/
	std::unique_ptr<ByteArrayGather> sendBufferForIO() override
	{
		auto conv = std::make_unique<SequentialByteArrayReaderWithLock>(std::move(m_sendBuffers));
		m_sendBuffers = std::vector<std::shared_ptr<ByteArray>>();
		return conv;
	}

	/**
	* @brief Write peer message to buffer
	* @return The ByteArray to iovec convertor
	*/
	std::unique_ptr<ByteArrayGather> receiveBufferForIO() override
	{
		return std::make_unique<ByteArrayWriterWithLock>(m_receiveArray);
	}

	ByteArray::LengthType bytesReceived() const override
	{
		return m_receiveArray->readableBytes();
	}

	ByteArray::LengthType bytesCanBeSend() const override
	{
		ByteArray::LengthType size = 0;
		for (auto &s : m_sendBuffers)
			size += s->readableBytes();
		return size;
	}

private:
	std::vector<std::shared_ptr<ByteArray>> m_sendBuffers;
	std::shared_ptr<ByteArray> m_receiveArray;
};

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
