#include "internal/handlers/TcpConnection.h"
#include "internal/support/Log.h"
#include "eventloop/EventLoop.h"
#include "channel/Channel.h"
#include "internal/socket/SocketIO.h"
#include "internal/socket/SocketEnums.h"
#include "error/Exception.h"
#include "error/SocketError.h"
#include "internal/socket/Socket.h"
#include "channel/TcpChannel.h"
#include "internal/buffer/ChannelBufferConversion.h"
#include "time/TickTimer.h"
#include "eventloop/EventLoopManager.h"

using std::make_unique;
using std::make_shared;

namespace netpp::internal::handlers {
TcpConnection::TcpConnection(eventloop::EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
							 Events eventsPrototype, ConnectionConfig config)
		: epoll::EventHandler(loop), m_state{socket::TcpState::Established},
		  m_isWaitWriting{false}, m_socket{std::move(socket)},
		  m_events{std::move(eventsPrototype)}, m_config{config}
{}

void TcpConnection::handleIn()
{
	try
	{
		renewWheel();
		socket::SocketIO::read(m_socket.get(), m_bufferConvertor->writeBufferConvert(m_connectionBufferChannel.get()));
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
		writeCompleted = socket::SocketIO::write(m_socket.get(), m_bufferConvertor->readBufferConvert(m_connectionBufferChannel.get()));
		if (writeCompleted)
		{
			m_epollEvent->deactivate(epoll::EpollEv::OUT);
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
	m_epollEvent->deactivate({epoll::EpollEv::RDHUP, epoll::EpollEv::IN});
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
				connection->m_epollEvent->deactivate(epoll::EpollEv::RDHUP);

				if (connection->m_config.enableAutoClose)
				{
					// force close connection in wheel
					connection->m_halfCloseWheel = std::make_unique<time::TickTimer>();
					connection->m_halfCloseWheel->setSingleShot(true);
					connection->m_halfCloseWheel->setInterval(connection->m_config.halfCloseConnectionTimeout);
					auto weakConnection = connection->weak_from_this();
					connection->m_halfCloseWheel->setOnTimeout([weakConnection] {
																   auto conn = weakConnection.lock();
																   if (conn)
																   {
																	   LOG_INFO("half closed connection timeout, force close");
																	   conn->forceClose();
																   }
															   }
					);
					connection->m_halfCloseWheel->start();
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
	if (m_idleConnectionWheel)
		m_idleConnectionWheel->restart();
	if (m_halfCloseWheel)
		m_halfCloseWheel->restart();
}

void TcpConnection::forceClose()
{
	std::weak_ptr<TcpConnection> connectionWeak = weak_from_this();
	_loopThisHandlerLiveIn->runInLoop([connectionWeak] {
		auto connection = connectionWeak.lock();
		if (connection)
		{
			LOG_TRACE("Force close socket {}", m_socket->fd());
			if (connection->m_idleConnectionWheel)
				connection->m_idleConnectionWheel->stop();
			if (connection->m_halfCloseWheel)
				connection->m_halfCloseWheel->stop();
			connection->m_events.onDisconnect(connection->m_connectionBufferChannel);
			connection->m_epollEvent->disable();
			connection->_loopThisHandlerLiveIn->removeEventHandlerFromLoop(connection);
			connection->m_state.store(socket::TcpState::Closed, std::memory_order_release);
		}
	});
}

int TcpConnection::connectionId()
{
	return m_socket->fd();
}

std::shared_ptr<TcpConnection> TcpConnection::makeTcpConnection(eventloop::EventLoop *loop, std::unique_ptr<socket::Socket> &&socket,
															  Events eventsPrototype, ConnectionConfig config)
{
	auto connection = std::make_shared<TcpConnection>(loop, std::move(socket), std::move(eventsPrototype), config);
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connection, connection->m_socket->fd());
	epoll::EpollEvent *eventPtr = event.get();
	connection->m_epollEvent = std::move(event);
	connection->m_connectionBufferChannel = std::make_shared<TcpChannel>(connection);
	connection->m_bufferConvertor = connection->m_connectionBufferChannel->createBufferConvertor();

	// set up events
	loop->addEventHandlerToLoop(connection);
	eventPtr->active({epoll::EpollEv::IN, epoll::EpollEv::RDHUP});

	if (connection->m_config.enableAutoClose)
	{
		// set up kick idle connection here
		connection->m_idleConnectionWheel = std::make_unique<time::TickTimer>();
		connection->m_idleConnectionWheel->setSingleShot(true);
		connection->m_idleConnectionWheel->setInterval(connection->m_config.idleConnectionTimeout);
		auto weakConnection = connection->weak_from_this();
		connection->m_idleConnectionWheel->setOnTimeout([weakConnection] {
															auto conn = weakConnection.lock();
															if (conn)
															{
																LOG_INFO("idle connection timeout, close write");
																conn->closeAfterWriteCompleted();
															}
														}
		);
		connection->m_idleConnectionWheel->start();
	}

	return connection;
}
}
