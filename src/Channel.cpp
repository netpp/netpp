//
// Created by gaojian on 2020/7/9.
//

#include "Channel.h"
#include "handlers/TcpConnection.h"

namespace netpp {
void Channel::send()
{
	auto connection = _connection.lock();
	if (connection)
		connection->sendInLoop();
}

void Channel::close()
{
	auto connection = _connection.lock();
	if (connection)
		connection->closeAfterWriteCompleted();
}
}
