//
// Created by gaojian on 22-7-18.
//

#include "channel/Channel.h"
#include "epoll/handlers/SocketConnectionHandler.h"

namespace netpp {
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