//
// Created by gaojian on 2020/7/9.
//

#include "Channel.h"
#include "handlers/TcpConnection.h"
#include "EventLoop.h"

namespace netpp {
void Channel::send()
{
	// Move to event loop thread
	auto connection = _connection.lock();
	if (connection)
	{
		// capture weak_ptr in case TcpConnection is destructed
		auto weakConnection = _connection;
		connection->getConnectionLoop()->runInLoop([weakConnection](){
			auto connection = weakConnection.lock();
			if (connection)
				connection->sendInLoop();
		});
	}
}

void Channel::close()
{
	// Move to event loop thread
	auto connection = _connection.lock();
	if (connection)
	{
		// capture weak_ptr in case TcpConnection is destructed
		auto weakConnection = _connection;
		connection->getConnectionLoop()->runInLoop([weakConnection]{
			auto connection = weakConnection.lock();
			if (connection)
				connection->closeAfterWriteCompleted();
		});
	}
}
}
