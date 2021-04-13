#include "internal/handlers/TcpConnection.h"
#include "internal/support/Log.h"
#include "EventLoop.h"
#include "Channel.h"
#include "internal/socket/SocketIO.h"
#include "internal/socket/SocketEnums.h"
#include "error/Exception.h"
#include "error/SocketError.h"
#include "internal/time/TimeWheel.h"
#include "ByteArray.h"
#include "internal/socket/Socket.h"

using std::make_unique;
using std::make_shared;

namespace netpp::internal::handlers {

/**
 * @brief Time wheel for connection who have not transfer any data in few time,
 * try to close it on timeout
 *
 */
class IdleConnectionWheelEntry : public internal::time::TimeWheelEntry {
public:
	explicit IdleConnectionWheelEntry(std::weak_ptr<TcpConnection> connection)
			: internal::time::TimeWheelEntry("idle"), _connection{std::move(connection)}
	{}

	~IdleConnectionWheelEntry() override = default;

	void onTimeout() override
	{
		auto conn = _connection.lock();
		if (conn)
		{
			LOG_INFO("idle connection timeout, close write");
			conn->closeAfterWriteCompleted();
		}
	}

private:
	std::weak_ptr<TcpConnection> _connection;
};

/**
 * @brief Time wheel for connection who's half closed(during Four-Way-Wavehand),
 * have not transfer any data in few time, force close it on timeout
 *
 */
class HalfCloseConnectionWheelEntry : public internal::time::TimeWheelEntry {
public:
	explicit HalfCloseConnectionWheelEntry(std::weak_ptr<TcpConnection> connection)
			: internal::time::TimeWheelEntry("half close"), _connection{std::move(connection)}
	{}

	~HalfCloseConnectionWheelEntry() override = default;

	void onTimeout() override
	{
		auto conn = _connection.lock();
		if (conn)
		{
			LOG_INFO("half closed connection timeout, force close");
			conn->forceClose();
		}
	}

private:
	std::weak_ptr<TcpConnection> _connection;
};

TcpConnection::TcpConnection(std::unique_ptr<socket::Socket> &&socket)
		: m_state{socket::TcpState::Established},
		  m_isWaitWriting{false}, m_socket{std::move(socket)},
		  m_writeBuffer{make_shared<ByteArray>()}, m_receiveBuffer{make_shared<ByteArray>()}
{}

void TcpConnection::handleIn()
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

void TcpConnection::handleOut()
{
	try
	{
		renewWheel();
		// may not write all data
		// TODO: handle read/write timeout
		if (socket::SocketIO::write(m_socket.get(), m_writeBuffer))
		{
			m_epollEvent->deactive(epoll::EpollEv::OUT);
			// TODO: do we need high/low watermark to notify?
			m_events.onWriteCompleted();
			m_isWaitWriting = false;
			if (m_state.load(std::memory_order_acquire) == socket::TcpState::Closing)
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

void TcpConnection::handleRdhup()
{
	LOG_TRACE("Socket {} rdhup", m_socket->fd());

	/**
	 * As close(fd) and shutdown(fd, SHUT_WR) notify as EPOLLIN and EPOLLRDHUP, we can not tell the difference
	 * 1. If we are active close, the connection state is set to Closing, when pear respond with close or shutdown,
	 * we will force close this connection
	 * 2. If we are passive close, wait util transfer done, then close the connection
	 */
	if (m_state.load(std::memory_order_acquire) == socket::TcpState::Closing)
		forceClose();
	else    // passive close
		closeAfterWriteCompleted();
}

void TcpConnection::sendInLoop()
{
	// Move to event loop thread
	// capture weak_ptr in case TcpConnection is destructed
	std::weak_ptr<TcpConnection> connectionWeak = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([connectionWeak] {
										  auto connection = connectionWeak.lock();
										  if (connection)
										  {
											  connection->m_isWaitWriting = true;
											  connection->m_epollEvent->active(epoll::EpollEv::OUT);
										  }
									  }
	);
}

void TcpConnection::closeAfterWriteCompleted()
{
	// Move to event loop thread
	// capture weak_ptr in case TcpConnection is destructed
	std::weak_ptr<TcpConnection> connectionWeak = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([connectionWeak] {
										  // nothing to write, close directly
										  auto connection = connectionWeak.lock();
										  if (connection)
										  {
											  connection->m_state.store(socket::TcpState::Closing, std::memory_order_release);
											  if (!connection->m_isWaitWriting)
												  connection->closeWrite();
										  }
									  }
	);
}

std::shared_ptr<Channel> TcpConnection::getIOChannel()
{
	return make_shared<Channel>(shared_from_this(), m_writeBuffer, m_receiveBuffer);
}

void TcpConnection::renewWheel()
{
	auto wheel = _loopThisHandlerLiveIn->getTimeWheel();
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
	auto wheel = _loopThisHandlerLiveIn->getTimeWheel();
	if (wheel)
	{
		auto halfCloseWheel = std::make_shared<HalfCloseConnectionWheelEntry>(shared_from_this());
		_halfCloseWheel = halfCloseWheel;
		wheel->addToWheel(halfCloseWheel);
	}
}

void TcpConnection::forceClose()
{
	LOG_TRACE("Force close socket {}", m_socket->fd());
	// no need to remove wheels, they will self destructed after timeout
	auto wheel = _loopThisHandlerLiveIn->getTimeWheel();
	if (wheel)
	{
		wheel->removeFromWheel(_idleConnectionWheel);
		wheel->removeFromWheel(_halfCloseWheel);
	}
	m_events.onDisconnect();
	m_epollEvent->disable();
	// extern TcpConnection life after remove
	volatile auto externLife = shared_from_this();
	_loopThisHandlerLiveIn->removeEventHandlerFromLoop(shared_from_this());
	m_state.store(socket::TcpState::Closed, std::memory_order_release);
}

std::weak_ptr<TcpConnection> TcpConnection::makeTcpConnection(EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
															  Events eventsPrototype)
{
	auto connection = std::make_shared<TcpConnection>(std::move(socket));
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connection, connection->m_socket->fd());
	epoll::EpollEvent *eventPtr = event.get();
	connection->m_epollEvent = std::move(event);
	connection->m_events = std::move(eventsPrototype);
	connection->_loopThisHandlerLiveIn = loop;
	// set up events
	loop->addEventHandlerToLoop(connection);
	eventPtr->active({epoll::EpollEv::IN, epoll::EpollEv::RDHUP});
	// set up kick idle connection here
	auto wheel = loop->getTimeWheel();
	if (wheel)
	{
		auto idleWheel = std::make_shared<IdleConnectionWheelEntry>(connection);
		connection->_idleConnectionWheel = idleWheel;
		wheel->addToWheel(idleWheel);
	}
	return connection;
}
}
