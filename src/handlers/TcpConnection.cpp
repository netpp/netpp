#include "handlers/TcpConnection.h"
#include "support/Log.h"
#include "EventLoop.h"
#include "Channel.h"
#include "socket/SocketIO.h"
#include "socket/SocketEnums.h"
#include "error/Exception.h"
#include "error/SocketError.h"
#include "time/TimeWheel.h"

using std::make_unique;
using std::make_shared;

namespace netpp::internal::handlers {

class IdelConnectionWheelEntry : public time::TimeWheelEntry {
public:
	explicit IdelConnectionWheelEntry(std::weak_ptr<TcpConnection> connection)
		: _connection{connection}
	{}
	~IdelConnectionWheelEntry() override = default;

	void onTimeout() override
	{
		LOG_INFO("idle connection timeout, close write");
		auto conn = _connection.lock();
		if (conn)
			conn->closeAfterWriteCompleted();
	}

private:
	std::weak_ptr<TcpConnection> _connection;
};

class HalfCloseConnectionWheelEntry : public time::TimeWheelEntry {
public:
	explicit HalfCloseConnectionWheelEntry(std::weak_ptr<TcpConnection> connection)
		: _connection{connection}
	{}
	~HalfCloseConnectionWheelEntry() override = default;

	void onTimeout() override
	{
		LOG_INFO("half closed connection timeout, close write");
		auto conn = _connection.lock();
		if (conn)
			conn->handleClose();
	}

private:
	std::weak_ptr<TcpConnection> _connection;
};

TcpConnection::TcpConnection(std::unique_ptr<socket::Socket> &&socket, EventLoop *loop)
		: _loop{loop}, m_state{socket::TcpState::Connected}, 
		m_isWaitWriting{false}, m_socket{std::move(socket)}, 
		m_writeBuffer{make_shared<ByteArray>()}, m_receiveBuffer{make_shared<ByteArray>()}
{}

void TcpConnection::handleRead()
{
	try
	{
		renewWheel();
		socket::SocketIO::read(m_socket.get(), m_receiveBuffer);
		LOG_TRACE("Available size {}", m_receiveBuffer->readableBytes());
		auto channel = make_shared<Channel>(shared_from_this(), m_writeBuffer, m_receiveBuffer);
		m_events.onMessageReceived(channel);
	}
	catch (error::SocketException &se)
	{
		// connection refused or not connected
		m_events.onError(se.getErrorCode());
		closeAfterWriteCompleted();
	}
	catch (error::ResourceLimitException &rle)
	{
		m_events.onError(rle.getSocketErrorCode());
	}
}

void TcpConnection::handleWrite()
{
	try
	{
		renewWheel();
		if (socket::SocketIO::write(m_socket.get(), m_writeBuffer))	// if write all
		{
			m_epollEvent->setEnableWrite(false);
			m_events.onWriteCompleted();
			m_isWaitWriting = false;
			if (m_state == socket::TcpState::Disconnecting)
				closeWrite();
		}
	}
	catch (error::SocketException &se)
	{
		// connection reset or not connect or connect shutdown
		m_events.onError(se.getErrorCode());
		m_isWaitWriting = false;
		closeAfterWriteCompleted();
	}
	catch (error::ResourceLimitException &rle)
	{
		m_events.onError(rle.getSocketErrorCode());
	}
}

void TcpConnection::handleError()
{
	LOG_ERROR("Socket {} error", m_socket->fd());
	m_events.onError(error::SocketError::E_EPOLLERR);
}

void TcpConnection::handleClose()
{
	LOG_TRACE("Socket {} disconnected", m_socket->fd());
	auto wheel = EventLoop::thisLoop()->getTimeWheel();
	if (wheel)
		wheel->removeFromWheel(_halfCloseWheel);
	m_events.onDisconnect();
	m_epollEvent->disableEvents();
	// extern TcpConnection life after remove
	volatile auto externLife = shared_from_this();
	EventLoop::thisLoop()->removeEventHandlerFromLoop(shared_from_this());
	m_state = socket::TcpState::Disconnected;
}

void TcpConnection::sendInLoop()
{
	// Move to event loop thread
	// capture weak_ptr in case TcpConnection is destructed
	std::weak_ptr<TcpConnection> connectionWeak = shared_from_this();
	_loop->runInLoop([connectionWeak]{
		auto connection = connectionWeak.lock();
		if (connection)
		{
			connection->m_isWaitWriting = true;
			connection->m_epollEvent->setEnableWrite(true);
		}
	});
}

void TcpConnection::closeAfterWriteCompleted()
{
	// Move to event loop thread
	// capture weak_ptr in case TcpConnection is destructed
	std::weak_ptr<TcpConnection> connectionWeak = shared_from_this();
	_loop->runInLoop([connectionWeak]{
		// nothing to write, close direcly
		auto connection = connectionWeak.lock();
		if (connection)
		{
			if (!connection->m_isWaitWriting)
				connection->closeWrite();
			connection->m_state = socket::TcpState::Disconnecting;
		}
	});
}

std::shared_ptr<Channel> TcpConnection::getIOChannel()
{
	return make_shared<Channel>(shared_from_this(), m_writeBuffer, m_receiveBuffer);
}

void TcpConnection::renewWheel()
{
	auto wheel = EventLoop::thisLoop()->getTimeWheel();
	if (wheel)
	{
		if (!_idleConnectionWheel.expired())
			wheel->renew(_idleConnectionWheel.lock());
		if (!_halfCloseWheel.expired())
			wheel->renew(_idleConnectionWheel.lock());
	}
}

void TcpConnection::closeWrite()
{
	m_socket->shutdownWrite();
	// force close connection in wheel
	auto wheel = EventLoop::thisLoop()->getTimeWheel();
	if (wheel)
	{
		auto halfCloseWheel = std::make_shared<HalfCloseConnectionWheelEntry>(shared_from_this());
		_halfCloseWheel = halfCloseWheel;
		wheel->addToWheel(halfCloseWheel);
	}
}

std::weak_ptr<TcpConnection> TcpConnection::makeTcpConnection(EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
												 Events eventsPrototype)
{
	auto connection = std::make_shared<TcpConnection>(std::move(socket), loop);
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connection, connection->m_socket->fd());
	epoll::EpollEvent *eventPtr = event.get();
	connection->m_epollEvent = std::move(event);
	connection->m_events = eventsPrototype;
	// set up events
	loop->addEventHandlerToLoop(connection);
	eventPtr->setEnableWrite(false);
	eventPtr->setEnableRead(true);
	// set up kick idle connection here
	auto wheel = loop->getTimeWheel();
	if (wheel)
	{
		auto idleWheel = std::make_shared<IdelConnectionWheelEntry>(connection);
		connection->_idleConnectionWheel = idleWheel;
		wheel->addToWheel(idleWheel);
	}
	return connection;
}
}
