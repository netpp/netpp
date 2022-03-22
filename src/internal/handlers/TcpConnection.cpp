#include "internal/handlers/TcpConnection.h"
#include "internal/support/Log.h"
#include "EventLoop.h"
#include "channel/Channel.h"
#include "internal/socket/SocketIO.h"
#include "internal/socket/SocketEnums.h"
#include "error/Exception.h"
#include "error/SocketError.h"
#include "internal/time/TimeWheel.h"
#include "ByteArray.h"
#include "internal/socket/Socket.h"
#include "channel/TcpChannel.h"

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
		  m_isWaitWriting{false}, m_socket{std::move(socket)}
{}

void TcpConnection::handleIn()
{
	try
	{
		renewWheel();
		socket::SocketIO::read(m_socket.get(), m_connectionBufferChannel->ioWriter());
		LOG_TRACE("Available size {}", m_receiveBuffer->readableBytes());
		m_events.onMessageReceived(m_connectionBufferChannel);
	}
	catch (error::SocketException &se)
	{
		// connection refused or not connected
		m_events.onError(se.getErrorCode());
		forceClose();
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
		// may not write all data this round, keep OUT event on and wait for next round
		bool writeCompleted;
		// TODO: handle read/write timeout
		writeCompleted = socket::SocketIO::write(m_socket.get(), m_connectionBufferChannel->ioReader());
		if (writeCompleted)
		{
			m_epollEvent->deactive(epoll::EpollEv::OUT);
			// TODO: do we need high/low watermark to notify?
			m_events.onWriteCompleted(m_connectionBufferChannel);
			m_isWaitWriting = false;
			// if write completed, and we are half-closing, force close this connection
			if (m_state.load(std::memory_order_acquire) == socket::TcpState::HalfClose)
				forceClose();
		}
	}
	catch (error::SocketException &se)
	{
		// connection reset or not connect or connect shutdown
		m_events.onError(se.getErrorCode());
		m_isWaitWriting = false;
		// if something wrong on this socket, force close
		forceClose();
	}
	catch (error::ResourceLimitException &rle)
	{
		m_events.onError(rle.getSocketErrorCode());
	}
}

void TcpConnection::handleRdhup()
{
	LOG_TRACE("Socket {} rdhup", m_socket->fd());

	// disable RDHUP and IN
	m_epollEvent->deactive(epoll::EpollEv::RDHUP);
	m_epollEvent->deactive(epoll::EpollEv::IN);
	socket::TcpState establishedState = socket::TcpState::Established;
	// transform state: Established->HalfClose
	if (m_state.compare_exchange_strong(establishedState, socket::TcpState::HalfClose, std::memory_order_acq_rel))
	{
		// if state changed to HalfClose, and no data to write, close immediately
		if (!m_isWaitWriting)
			forceClose();
	}
}

void TcpConnection::sendInLoop()
{
	// Move to event loop thread
	// capture weak_ptr in case TcpConnection destructed
	std::weak_ptr<TcpConnection> connectionWeak = weak_from_this();
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
	// capture weak_ptr in case TcpConnection destructed
	std::weak_ptr<TcpConnection> connectionWeak = weak_from_this();
	_loopThisHandlerLiveIn->runInLoop([connectionWeak] {
		// nothing to write, close directly
		auto connection = connectionWeak.lock();
		if (connection)
		{
			// Established->ActiveClosing
			socket::TcpState onlyChangeInEstablished = socket::TcpState::Established;
			if (connection->m_state.compare_exchange_strong(
					onlyChangeInEstablished,
					socket::TcpState::HalfClose,
					std::memory_order_acq_rel) && !connection->m_isWaitWriting)
			{
				connection->m_socket->shutdownWrite();
				// disable RDHUP, shutdown write will wake up epoll_wait with EPOLLRDHUP and EPOLLIN
				connection->m_epollEvent->deactive(epoll::EpollEv::RDHUP);
				// force close connection in wheel
				auto wheel = connection->_loopThisHandlerLiveIn->getTimeWheel();
				if (wheel)
				{
					auto halfCloseWheel = std::make_shared<HalfCloseConnectionWheelEntry>(connection);
					connection->_halfCloseWheel = halfCloseWheel;
					wheel->addToWheel(halfCloseWheel);
				}
			}
		}
	});
}

std::shared_ptr<Channel> TcpConnection::getIOChannel()
{
	return m_connectionBufferChannel;
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
	m_events.onDisconnect(m_connectionBufferChannel);
	m_epollEvent->disable();
	// extern TcpConnection life after remove
	volatile auto externLife = shared_from_this();
	_loopThisHandlerLiveIn->removeEventHandlerFromLoop(shared_from_this());
	m_state.store(socket::TcpState::Closed, std::memory_order_release);
}

int TcpConnection::connectionId()
{
	return m_socket->fd();
}

std::weak_ptr<TcpConnection> TcpConnection::makeTcpConnection(EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
															  Events eventsPrototype)
{
	auto connection = std::make_shared<TcpConnection>(std::move(socket));
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connection, connection->m_socket->fd());
	epoll::EpollEvent *eventPtr = event.get();
	connection->m_epollEvent = std::move(event);
	connection->m_connectionBufferChannel = std::make_shared<TcpChannel>(connection);
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
