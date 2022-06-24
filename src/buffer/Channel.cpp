//
// Created by gaojian on 2020/7/9.
//

#include "buffer/Channel.h"
#include "epoll/handlers/SocketConnectionHandler.h"
#include "support/Types.h"

namespace netpp {
Channel::Channel(std::weak_ptr<SocketConnectionHandler> connection)
		: _connection{std::move(connection)}
{}

Channel::~Channel() = default;

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

bool Channel::channelActive() const
{
	// writing to a connection only available when state is Established
	auto connection = _connection.lock();
	return (connection) && (connection->currentState() == TcpState::Established);
}
}
