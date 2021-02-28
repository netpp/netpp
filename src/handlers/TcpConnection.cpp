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

/**
 * @brief Time wheel for connection who have not transfer any data in few time,
 * try to close it on timeout
 * 
 */
class IdelConnectionWheelEntry : public internal::time::TimeWheelEntry {
public:
	explicit IdelConnectionWheelEntry(std::weak_ptr<TcpConnection> connection)
		: internal::time::TimeWheelEntry("idle"), _connection{connection}
	{}
	~IdelConnectionWheelEntry() override = default;

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
		: internal::time::TimeWheelEntry("hanlf close"), _connection{connection}
	{}
	~HalfCloseConnectionWheelEntry() override = default;

	void onTimeout() override
	{
		auto conn = _connection.lock();
		if (conn)
		{
			LOG_INFO("half closed connection timeout, force close");
			conn->handleRdhup();
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
		if (socket::SocketIO::write(m_socket.get(), m_writeBuffer))
		{
			m_epollEvent->deactive(epoll::Event::OUT);
			m_events.onWriteCompleted();
			m_isWaitWriting = false;
			if (m_state == socket::TcpState::Closing)
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

void TcpConnection::handleErr()
{
	LOG_ERROR("Socket {} error", m_socket->fd());
	m_events.onError(error::SocketError::E_EPOLLERR);
}

void TcpConnection::handleRdhup()
{
	LOG_TRACE("Socket {} disconnected", m_socket->fd());
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
	m_state = socket::TcpState::Closed;
}

void TcpConnection::sendInLoop()
{
	// Move to event loop thread
	// capture weak_ptr in case TcpConnection is destructed
	std::weak_ptr<TcpConnection> connectionWeak = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([connectionWeak]{
		auto connection = connectionWeak.lock();
		if (connection)
		{
			connection->m_isWaitWriting = true;
			connection->m_epollEvent->active(epoll::Event::OUT);
		}
	});
}

void TcpConnection::closeAfterWriteCompleted()
{
	// Move to event loop thread
	// capture weak_ptr in case TcpConnection is destructed
	std::weak_ptr<TcpConnection> connectionWeak = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([connectionWeak]{
		// nothing to write, close direcly
		auto connection = connectionWeak.lock();
		if (connection)
		{
			if (!connection->m_isWaitWriting)
				connection->closeWrite();
			connection->m_state = socket::TcpState::Closing;
		}
	});
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

std::weak_ptr<TcpConnection> TcpConnection::makeTcpConnection(EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
												 Events eventsPrototype)
{
	auto connection = std::make_shared<TcpConnection>(std::move(socket));
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connection, connection->m_socket->fd());
	epoll::EpollEvent *eventPtr = event.get();
	connection->m_epollEvent = std::move(event);
	connection->m_events = eventsPrototype;
	connection->_loopThisHandlerLiveIn = loop;
	// set up events
	loop->addEventHandlerToLoop(connection);
	eventPtr->deactive(epoll::Event::OUT);
	eventPtr->active({epoll::Event::IN, epoll::Event::ERR, epoll::Event::RDHUP});
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
