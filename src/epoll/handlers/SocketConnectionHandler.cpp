#include "epoll/handlers/SocketConnectionHandler.h"

#include <utility>
#include "support/Log.h"
#include "eventloop/EventLoop.h"
#include "error/Exception.h"
#include "buffer/TcpBuffer.h"
#include "time/TickTimer.h"
#include "iodevice/SocketDevice.h"
#include "buffer/BufferIOConversion.h"
#include "channel/TcpChannel.h"

using std::make_unique;
using std::make_shared;

namespace netpp {
SocketConnectionHandler::SocketConnectionHandler(EventLoop *loop, std::unique_ptr<SocketDevice> &&socket,
												 std::shared_ptr<Channel> channelToBind, std::shared_ptr<Buffer> buffer)
		: EpollEventHandler(loop), m_state{TcpState::Established},
		  m_isWaitWriting{false}, m_socket{std::move(socket)},
		  m_idleTimeInterval{-1}
{
	m_bindChannel = std::move(channelToBind);
	m_connectionBuffer = std::move(buffer);

	// set up events
	activeEvents(EpollEv::IN | EpollEv::RDHUP);
}

SocketConnectionHandler::~SocketConnectionHandler() = default;

int SocketConnectionHandler::fileDescriptor() const
{
	return m_socket->fileDescriptor();
}

void SocketConnectionHandler::setIdleTimeout(TimerInterval idleTime)
{
	_loopThisHandlerLiveIn->runInLoop([this, connectionWeak(weak_from_this()), idleTime] {
		auto connection = connectionWeak.lock();
		if (connection)
		{
			m_idleTimeInterval = idleTime;
			if (m_idleTimeInterval != -1)
			{
				// set up kick idle connection here
				m_idleConnectionWheel = std::make_unique<TickTimer>();
				m_idleConnectionWheel->setSingleShot(true);
				m_idleConnectionWheel->setInterval(m_idleTimeInterval);
				m_idleConnectionWheel->setOnTimeout([weakConnection{weak_from_this()}] {
														auto conn = weakConnection.lock();
														if (conn)
														{
															LOG_INFO("idle connection timeout, close write");
															conn->closeAfterWriteCompleted();
														}
													}
				);
				m_idleConnectionWheel->start();
			}
		}
	});
}

void SocketConnectionHandler::handleIn()
{
	try
	{
		renewWheel();
		m_socket->read(m_connectionBuffer);
		LOG_TRACE("Available size {}", m_receiveBuffer->readableBytes());
		if (m_receivedCallback)
			m_receivedCallback(m_bindChannel);
	}
	catch (InternalException &e)
	{
		Error errorCode = e.getErrorCode();
		LOG_WARN("Receive message error {}", getErrorDescription(errorCode));
		switch (errorCode)
		{
			case Error::InvalidAddress:
			case Error::BadFD:
			case Error::ConnectRefused:
				LOG_WARN("Unrecoverable error on {}, force close", m_socket->fileDescriptor());
				forceClose();
				break;
			case Error::MemoryUnavailable:
			default:
				break;
		}
		if (m_errorCallback)
			m_errorCallback(errorCode);
		else
			throw e;
	}
}

void SocketConnectionHandler::handleOut()
{
	try
	{
		renewWheel();
		// may not write all data this round, keep OUT event on and wait for next round
		// TODO: handle read/write timeout
		bool writeCompleted = m_socket->write(m_connectionBuffer);
		if (writeCompleted)
		{
			deactivateEvents(EpollEv::OUT);
			// TODO: do we need high/low watermark to notify?
			if (m_writeCompletedCallback)
				m_writeCompletedCallback(m_bindChannel);
			m_isWaitWriting = false;
			// if write completed, and we are half-closing, force close this connection
			if (m_state.load(std::memory_order_acquire) == TcpState::HalfClose)
				forceClose();
		}
	}
	catch (InternalException &e)
	{
		Error errorCode = e.getErrorCode();
		LOG_WARN("Send message error on {} reason {}", m_socket->fileDescriptor(), getErrorDescription(errorCode));
		switch (errorCode)
		{
			case Error::InvalidAddress:
			case Error::BadFD:
			case Error::BrokenConnection:
				m_isWaitWriting = false;
				LOG_WARN("Unrecoverable error on {}, force close", m_socket->fileDescriptor());
				forceClose();
				break;
			case Error::MemoryUnavailable:
			default:
				break;
		}
		if (m_errorCallback)
			m_errorCallback(errorCode);
		else
			throw e;
	}
}

void SocketConnectionHandler::handleRdhup()
{
	LOG_TRACE("Socket {} rdhup", m_socket->fileDescriptor());

	// disable RDHUP and IN
	deactivateEvents(EpollEv::RDHUP | EpollEv::IN);
	TcpState establishedState = TcpState::Established;
	// transform state: Established->HalfClose
	if (m_state.compare_exchange_strong(establishedState, TcpState::HalfClose, std::memory_order_acq_rel))
	{
		// if state changed to HalfClose, and no data to write, close immediately
		if (!m_isWaitWriting)
			forceClose();
	}
}

void SocketConnectionHandler::sendInLoop()
{
	// Move to event loop thread
	// capture weak_ptr in case SocketConnectionHandler destructed
	_loopThisHandlerLiveIn->runInLoop([this, connectionWeak(weak_from_this())] {
										  auto connection = connectionWeak.lock();
										  if (connection)
										  {
											  m_isWaitWriting = true;
											  activeEvents(EpollEv::OUT);
										  }
									  }
	);
}

void SocketConnectionHandler::closeAfterWriteCompleted()
{
	// Move to event loop thread
	// capture weak_ptr in case SocketConnectionHandler destructed
	std::weak_ptr<SocketConnectionHandler> connectionWeak = weak_from_this();
	_loopThisHandlerLiveIn->runInLoop([this, connectionWeak{weak_from_this()}] {
		try
		{
			// nothing to write, close directly
			auto connection = connectionWeak.lock();
			if (connection)
			{
				// Established->ActiveClosing
				TcpState onlyChangeInEstablished = TcpState::Established;
				if (m_state.compare_exchange_strong(
						onlyChangeInEstablished,
						TcpState::HalfClose,
						std::memory_order_acq_rel
				) && !m_isWaitWriting)
				{
					m_socket->shutdownWrite();

					// disable RDHUP, shutdown write will wake up epoll_wait with EPOLLRDHUP and EPOLLIN
					deactivateEvents(EpollEv::RDHUP);

					if (m_idleTimeInterval != -1)
					{
						// force close connection in wheel
						m_halfCloseWheel = std::make_unique<TickTimer>();
						m_halfCloseWheel->setSingleShot(true);
						m_halfCloseWheel->setInterval(m_idleTimeInterval);
						m_halfCloseWheel->setOnTimeout([weakConnection{weak_from_this()}] {
																	   auto conn = weakConnection.lock();
																	   if (conn)
																	   {
																		   LOG_INFO("half closed connection timeout, force close");
																		   conn->forceClose();
																	   }
																   }
						);
						m_halfCloseWheel->start();
					}
				}
			}
		} catch (InternalException &e) {
			auto errorCode = e.getErrorCode();
			if (errorCode == Error::BadFD)
				LOG_WARN("Failed to shutdown write, reason {}", getErrorDescription(errorCode));
			else
				LOG_WARN("Unexpected exception {}", getErrorDescription(errorCode));
		}
	});
}

std::shared_ptr<Channel> SocketConnectionHandler::getIOChannel()
{
	return m_bindChannel;
}

void SocketConnectionHandler::setMessageReceivedCallBack(const MessageReceivedCallBack &cb)
{
	_loopThisHandlerLiveIn->runInLoop([this, connection{shared_from_this()}, cb] {
		m_receivedCallback = cb;
	});
}

void SocketConnectionHandler::setWriteCompletedCallBack(const WriteCompletedCallBack &cb)
{
	_loopThisHandlerLiveIn->runInLoop([this, connection{shared_from_this()}, cb] {
		m_writeCompletedCallback = cb;
	});
}

void SocketConnectionHandler::setDisconnectedCallBack(const DisconnectedCallBack &cb)
{
	_loopThisHandlerLiveIn->runInLoop([this, connection{shared_from_this()}, cb] {
		m_disconnectedCallback = cb;
	});
}

void SocketConnectionHandler::setErrorCallBack(const ErrorCallBack &cb)
{
	_loopThisHandlerLiveIn->runInLoop([this, connection{shared_from_this()}, cb] {
		m_errorCallback = cb;
	});
}

void SocketConnectionHandler::renewWheel()
{
	if (m_idleConnectionWheel)
		m_idleConnectionWheel->restart();
	if (m_halfCloseWheel)
		m_halfCloseWheel->restart();
}

void SocketConnectionHandler::forceClose()
{
	if (m_state.load(std::memory_order_acquire) != TcpState::Closed)
	{
		// catch shared ptr to extend life
		_loopThisHandlerLiveIn->runInLoop([this, connection{shared_from_this()}] {
											  LOG_TRACE("Force close socket {}", m_socket->fd());
											  disableEvent();
											  if (m_idleConnectionWheel)
												  m_idleConnectionWheel->stop();
											  if (m_halfCloseWheel)
												  m_halfCloseWheel->stop();
											  if (m_disconnectedCallback)
												  m_disconnectedCallback(m_bindChannel);
											  m_state.store(TcpState::Closed, std::memory_order_release);
											  _loopThisHandlerLiveIn->removeEventHandlerFromLoop(connection);
										  }
		);
	}
}
}
