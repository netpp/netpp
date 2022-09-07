//
// Created by gaojian on 22-7-18.
//

#include "channel/Channel.h"
#include "epoll/handlers/SocketConnectionHandler.h"
#include "buffer/TransferBuffer.h"

namespace netpp {

void Channel::send(const ByteArray &data)
{
	auto buffer = _buffer.lock();
	if (buffer)
	{
		buffer->write(data);
		auto connection = _connection.lock();
		if (connection)
			connection->sendInLoop();
	}
}

ByteArray::LengthType Channel::readableBytes() const
{
	auto buffer = _buffer.lock();
	if (buffer)
		return buffer->bytesReceived();
	return 0;
}

ByteArray Channel::peek(ByteArray::LengthType size)
{
	auto buffer = _buffer.lock();
	if (buffer)
		return buffer->peek(size);
	return {};
}

ByteArray Channel::read(ByteArray::LengthType size)
{
	auto buffer = _buffer.lock();
	if (buffer)
		return buffer->read(size);
	return {};
}

void Channel::close()
{
	auto connection = _connection.lock();
	if (connection)
		connection->closeAfterWriteCompleted();
}

void Channel::setMessageReceivedCallBack(const MessageReceivedCallBack &cb)
{
	auto connection = _connection.lock();
	if (connection)
		connection->setMessageReceivedCallBack(cb);
}

void Channel::setWriteCompletedCallBack(const WriteCompletedCallBack &cb)
{
	auto connection = _connection.lock();
	if (connection)
		connection->setWriteCompletedCallBack(cb);
}

void Channel::setDisconnectedCallBack(const DisconnectedCallBack &cb)
{
	auto connection = _connection.lock();
	if (connection)
		connection->setDisconnectedCallBack(cb);
}

void Channel::setErrorCallBack(const ErrorCallBack &cb)
{
	auto connection = _connection.lock();
	if (connection)
		connection->setErrorCallBack(cb);
}

void Channel::setIdleTimeout(TimerInterval idleTime)
{
	auto connection = _connection.lock();
	if (connection)
		connection->setIdleTimeout(idleTime);
}
} // netpp