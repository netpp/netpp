#include <cassert>
#include "epoll/handlers/SocketConnectorHandler.h"
#include "support/Log.h"
#include "epoll/handlers/SocketConnectionHandler.h"
#include "eventloop/EventLoop.h"
#include "eventloop/EventLoopManager.h"
#include "error/Exception.h"
#include "location/Address.h"
#include "time/Timer.h"
#include "Application.h"
#include "iodevice/TcpSocket.h"
#include "support/Util.h"
#include "channel/TcpChannel.h"

using std::make_unique;
using std::make_shared;

namespace netpp {
SocketConnectorHandler::SocketConnectorHandler(EventLoop *loop)
		: EpollEventHandler(loop), m_state{TcpState::Closed},
		  m_connectionEstablished{false}
{
}

SocketConnectorHandler::~SocketConnectorHandler() = default;

void SocketConnectorHandler::connect(const Address &address)
{
	m_connectionEstablished.store(false, std::memory_order_relaxed);
	_loopThisHandlerLiveIn->runInLoop([this, weakConnector{weak_from_this()}, address] {
		try
		{
			auto connector = weakConnector.lock();
			if (connector)
			{
				if (m_socket)	// another socket exist
					return;
				m_address = address;
				m_socket = std::make_unique<TcpSocket>();
				m_socket->open();
				if (m_socket->connect(m_address))    // connect success immediately
					handleOut();
				else
				{
					// connect in progress, manually enable read
					activeEvents(EpollEv::OUT);
					m_state = TcpState::Connecting;
				}
			}
		}
		catch (InternalException &e)
		{
			if (m_errorCallback)
				m_errorCallback(e.getErrorCode());
			else
				throw e;
		}
	}
	);
}

void SocketConnectorHandler::stopConnect()
{
	if (m_state != TcpState::Connecting)
		return;

	m_connectionEstablished.store(false, std::memory_order_relaxed);
	_loopThisHandlerLiveIn->runInLoop([this, weakConnector{weak_from_this()}] {
										  auto connector = weakConnector.lock();
										  // if connected, SocketConnectorHandler was already cleaned, no need to remove again
										  if (connector && m_state == TcpState::Connecting)
										  {
											  if (m_retryTimer)    // stop timer
											  {
												  m_retryTimer->stop();
												  m_retryTimer = nullptr;
											  }
											  disableEvent();
											  m_socket = nullptr;
											  m_state = TcpState::Closed;
										  }
									  }
	);
}

void SocketConnectorHandler::setConnectedCallBack(const ConnectedCallBack &cb)
{
	_loopThisHandlerLiveIn->runInLoop([this, connection{shared_from_this()}, cb] {
		m_connectedCallback = cb;
	});
}

void SocketConnectorHandler::setErrorCallBack(const ErrorCallBack &cb)
{
	_loopThisHandlerLiveIn->runInLoop([this, connection{shared_from_this()}, cb] {
		m_errorCallback = cb;
	});
}

void SocketConnectorHandler::handleOut()
{
	LOG_TRACE("Connector write available");
	SocketState state = m_socket->state();
	// todo maybe can retry in other situations
	if (state == SocketState::E_TimeOut || state == SocketState::E_ConnectRefused)
	{
		LOG_ERROR("Connector error {}", error::errorAsString(err));
		if (!m_retryTimer)
		{
			m_retryTimer = make_unique<Timer>(_loopThisHandlerLiveIn);
			m_retryTimer->setOnTimeout([this, weakConnector{weak_from_this()}] {
										   auto connector = weakConnector.lock();
										   if (connector)
										   {
											   disableEvent();
											   m_socket = std::make_unique<TcpSocket>();
											   m_socket->open();
											   connect(m_address);
										   }
									   }
			);
			m_retryTimer->setSingleShot(false);
			m_retryTimer->setInterval(1000);
			m_retryTimer->start();
		}
		else
		{
			TimerInterval currentInterval = m_retryTimer->interval();
			if (currentInterval < 4000)
				m_retryTimer->setInterval(currentInterval * 2);
			LOG_INFO("Connector error on fd {}, retry in {} ms", m_socket->fd(), currentInterval);
		}
	}
	else if (state == SocketState::E_InProgress)
	{
		LOG_TRACE("connecting, wait for connect done");
	}
	else if (state == SocketState::E_Ok)
	{
		if (m_retryTimer)    // stop timer
		{
			m_retryTimer->stop();
			m_retryTimer = nullptr;
		}
		APPLICATION_INSTANCE_REQUIRED();
		auto targetLoop = Application::loopManager()->dispatch();
		std::shared_ptr<Channel> channel = std::make_shared<TcpChannel>(targetLoop, std::move(m_socket));
		m_socket = nullptr;
		m_connectionEstablished.store(true, std::memory_order_relaxed);
		LOG_TRACE("Connected to server");
		m_state = TcpState::Established;
		m_connectedCallback(channel);
	}
}

int SocketConnectorHandler::fileDescriptor() const
{
	if (m_socket)
		m_socket->fileDescriptor();
	return -1;
}
}
