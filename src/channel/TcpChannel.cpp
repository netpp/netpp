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
	TcpChannelImpl(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket);
	~TcpChannelImpl() override;

	void send(const ByteArray &data) override;

	void close() override;

	void setMessageReceivedCallBack(const MessageReceivedCallBack &cb) override;
	void setWriteCompletedCallBack(const WriteCompletedCallBack &cb) override;
	void setDisconnectedCallBack(const DisconnectedCallBack &cb) override;
	void setErrorCallBack(const ErrorCallBack &cb) override;

	void init();

private:
	std::weak_ptr<Buffer> _buffer;
	std::weak_ptr<SocketConnectionHandler> _connection;
	EventLoop *_loop;
	std::unique_ptr<SocketDevice> &&_tmpSocketDev;
};

TcpChannelImpl::TcpChannelImpl(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket)
	: _loop{loop}, _tmpSocketDev{std::move(socket)}
{}

TcpChannelImpl::~TcpChannelImpl() = default;

void TcpChannelImpl::init()
{
	auto buffer = std::make_shared<TcpBuffer>();
	_buffer = buffer;
	auto connection = std::make_shared<SocketConnectionHandler>(_loop, std::move(_tmpSocketDev));
	_connection = connection;
	connection->init(shared_from_this(), buffer);
	_loop->addEventHandlerToLoop(connection);
}

void TcpChannelImpl::send(const ByteArray &data)
{
	auto buffer = _buffer.lock();
	if (buffer)
	{
		buffer->addWriteBuffer(std::make_shared<ByteArray>(data));
		auto connection = _connection.lock();
		if (connection)
			connection->sendInLoop();
	}
}

void TcpChannelImpl::close()
{
	auto connection = _connection.lock();
	if (connection)
		connection->closeAfterWriteCompleted();
}

void TcpChannelImpl::setMessageReceivedCallBack(const MessageReceivedCallBack &cb)
{
	auto connection = _connection.lock();
	if (connection)
		connection->setMessageReceivedCallBack(cb);
}

void TcpChannelImpl::setWriteCompletedCallBack(const WriteCompletedCallBack &cb)
{
	auto connection = _connection.lock();
	if (connection)
		connection->setWriteCompletedCallBack(cb);
}

void TcpChannelImpl::setDisconnectedCallBack(const DisconnectedCallBack &cb)
{
	auto connection = _connection.lock();
	if (connection)
		connection->setDisconnectedCallBack(cb);
}

void TcpChannelImpl::setErrorCallBack(const ErrorCallBack &cb)
{
	auto connection = _connection.lock();
	if (connection)
		connection->setErrorCallBack(cb);
}

TcpChannel::TcpChannel(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket)
	: m_impl{std::make_shared<TcpChannelImpl>(loop, std::move(socket))}
{
	m_impl->init();
}

TcpChannel::~TcpChannel() = default;


void TcpChannel::send(const ByteArray &data)
{
	m_impl->send(data);
}

void TcpChannel::close()
{
	m_impl->close();
}

void TcpChannel::setMessageReceivedCallBack(const MessageReceivedCallBack &cb)
{
	m_impl->setMessageReceivedCallBack(cb);
}

void TcpChannel::setWriteCompletedCallBack(const WriteCompletedCallBack &cb)
{
	m_impl->setWriteCompletedCallBack(cb);
}

void TcpChannel::setDisconnectedCallBack(const DisconnectedCallBack &cb)
{
	m_impl->setDisconnectedCallBack(cb);
}

void TcpChannel::setErrorCallBack(const ErrorCallBack &cb)
{
	m_impl->setErrorCallBack(cb);
}

}
