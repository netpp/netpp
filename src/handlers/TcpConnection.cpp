#include "handlers/TcpConnection.h"
#include "Log.h"
#include "EventLoop.h"
#include "Channel.h"
#include "socket/SocketIO.h"
#include "socket/SocketEnums.h"
#include "error/Exception.h"
#include "error/SocketError.h"
#include "time/TimeWheel.h"

using std::make_unique;
using std::make_shared;

namespace netpp::handlers {

class IdelConnectionWheelEntry : public time::TimeWheelEntry {
public:
	explicit IdelConnectionWheelEntry(std::weak_ptr<TcpConnection> connection)
		: _connection{connection}
	{}

	~IdelConnectionWheelEntry() override
	{
		SPDLOG_LOGGER_INFO(logger, "idle connection timeout, close write");
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

	~HalfCloseConnectionWheelEntry() override
	{
		SPDLOG_LOGGER_INFO(logger, "half closed connection timeout, close write");
		auto conn = _connection.lock();
		if (conn)
			conn->handleClose();
	}

private:
	std::weak_ptr<TcpConnection> _connection;
};

TcpConnection::TcpConnection(std::unique_ptr<socket::Socket> &&socket, EventLoop *loop) noexcept
		: EventHandler(socket->fd()), _loop{loop}, m_state{socket::TcpState::Connected}, 
		m_isWaitWriting{false}, m_socket{std::move(socket)}, 
		m_writeBuffer{make_shared<ByteArray>()}, m_receiveBuffer{make_shared<ByteArray>()}
{}

void TcpConnection::handleRead() noexcept
{
	try {
		renewWheel();
		socket::SocketIO::read(m_socket.get(), m_receiveBuffer);
		SPDLOG_LOGGER_TRACE(logger, "Available size {}", m_receiveBuffer->readableBytes());
		auto channel = make_shared<Channel>(shared_from_this(), m_writeBuffer, m_receiveBuffer);
		m_events->onMessageReceived(channel);
	} catch (error::SocketException &se) {
		// connection refused or not connected
		m_events->onError(se.getErrorCode());
		closeAfterWriteCompleted();
	} catch (error::ResourceLimitException &rle) {
		m_events->onError(rle.getErrorCode());
	}
}

void TcpConnection::handleWrite() noexcept
{
	try {
		renewWheel();
		if (socket::SocketIO::write(m_socket.get(), m_writeBuffer))	// if write all
		{
			m_epollEvent->setEnableWrite(false);
			m_events->onWriteCompleted();
			m_isWaitWriting = false;
			if (m_state == socket::TcpState::Disconnecting)
				closeWrite();
		}
	} catch (error::SocketException &se) {
		// connection reset or not connect or connect shutdown
		m_events->onError(se.getErrorCode());
		m_isWaitWriting = false;
		closeAfterWriteCompleted();
	} catch (error::ResourceLimitException &rle) {
		m_events->onError(rle.getErrorCode());
	}
}

void TcpConnection::handleError() noexcept
{
	SPDLOG_LOGGER_ERROR(logger, "Socket {} error", m_socket->fd());
	m_events->onError(error::SocketError::E_EPOLLERR);
}

// FIXME: did not call this if pair close write
void TcpConnection::handleClose() noexcept
{
	SPDLOG_LOGGER_TRACE(logger, "Socket {} disconnected", m_socket->fd());
	m_events->onDisconnect();
	m_epollEvent->disableEvents();
	// extern TcpConnection life after remove
	volatile auto externLife = shared_from_this();
	EventLoop::thisLoop()->removeEventHandlerFromLoop(shared_from_this());
	m_state = socket::TcpState::Disconnected;
}

void TcpConnection::sendInLoop() noexcept
{
	m_isWaitWriting = true;
	m_epollEvent->setEnableWrite(true);
}

void TcpConnection::closeAfterWriteCompleted() noexcept
{
	// nothing to write, close direcly
	if (!m_isWaitWriting)
		closeWrite();
	m_state = socket::TcpState::Disconnecting;
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

std::shared_ptr<Channel> TcpConnection::makeTcpConnection(EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
												 std::unique_ptr<support::EventInterface> &&eventsPrototype) noexcept
{
	auto connection = std::make_shared<TcpConnection>(std::move(socket), loop);
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connection);
	epoll::EpollEvent *eventPtr = event.get();
	connection->m_epollEvent = std::move(event);
	connection->m_events = std::move(eventsPrototype);
	// set up events
	eventPtr->setEnableWrite(false);
	eventPtr->setEnableRead(true);
	loop->addEventHandlerToLoop(connection);
	// set up kick idle connection here
	auto wheel = loop->getTimeWheel();
	if (wheel)
	{
		auto idleWheel = std::make_shared<IdelConnectionWheelEntry>(connection);
		connection->_idleConnectionWheel = idleWheel;
		wheel->addToWheel(idleWheel);
	}
	return make_shared<Channel>(connection, connection->m_writeBuffer, connection->m_receiveBuffer);
}
}
