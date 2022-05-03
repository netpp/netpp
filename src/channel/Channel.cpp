//
// Created by gaojian on 2020/7/9.
//

#include "channel/Channel.h"
#include "internal/handlers/TcpConnection.h"
#include "internal/socket/SocketEnums.h"

namespace netpp {
Channel::Channel(std::weak_ptr<internal::handlers::TcpConnection> connection)
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
	return (connection) && (connection->currentState() == internal::socket::TcpState::Established);
}

int Channel::channelId()
{
	auto connection = _connection.lock();
	if (connection)
		return connection->connectionId();
	else
		return -1;
}

}
