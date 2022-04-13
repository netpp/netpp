#include "internal/handlers/Connector.h"
#include "internal/support/Log.h"
#include "internal/handlers/TcpConnection.h"
#include "eventloop/EventLoop.h"
#include "eventloop/EventLoopManager.h"
#include "error/Exception.h"
#include "error/SocketError.h"
#include "Address.h"
#include "internal/socket/Socket.h"
#include "time/Timer.h"
#include "internal/socket/SocketEnums.h"
#include "Application.h"

using std::make_unique;
using std::make_shared;

namespace netpp::internal::handlers {
Connector::Connector(eventloop::EventLoop *loop, std::unique_ptr<socket::Socket> &&socket)
		: epoll::EventHandler(loop), m_socket{std::move(socket)}, m_state{socket::TcpState::Closed}, m_connectionEstablished{false}
{}

Connector::~Connector() = default;

void Connector::connect()
{
	auto connector = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([connector]{
		try
		{
			if (connector->m_state == socket::TcpState::Closed)
			{
				connector->_loopThisHandlerLiveIn->addEventHandlerToLoop(connector);
				// connect may success immediately, manually enable read
				connector->m_epollEvent->active(epoll::EpollEv::OUT);
				connector->m_socket->connect();
				connector->m_state = socket::TcpState::Connecting;
			}
		}
		catch (error::SocketException &se)
		{
			error::SocketError code = se.getErrorCode();
			if (code != error::SocketError::E_INPROGRESS)
				connector->m_events.onError(code);
		}
	});
}

void Connector::stopConnect()
{
	auto connector = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([connector]{
		// if connected, Connector was already cleaned, no need to remove again
		if (connector->m_state == socket::TcpState::Connecting)
		{
			if (connector->m_retryTimer)	// stop timer
			{
				connector->m_retryTimer->stop();
				connector->m_retryTimer = nullptr;
			}
			connector->m_epollEvent->disable();
			connector->_loopThisHandlerLiveIn->removeEventHandlerFromLoop(connector);
			connector->m_state = socket::TcpState::Closed;
		}
	});
}

void Connector::handleOut()
{
	LOG_TRACE("Connector write available");
	error::SocketError err = m_socket->getError();
	// todo maybe can retry in other situations
	if (err == error::SocketError::E_TIMEDOUT || err == error::SocketError::E_CONNREFUSED)
	{
		LOG_ERROR("Connector error {}", error::errorAsString(err));
		if (!m_retryTimer)
		{
			m_retryTimer = make_unique<netpp::time::Timer>(_loopThisHandlerLiveIn);
			m_retryTimer->setOnTimeout([this]{
				auto oldSocket = std::move(m_socket);
				m_socket = make_unique<socket::Socket>(oldSocket->getAddr());
				m_epollEvent->disable();
				m_epollEvent = make_unique<epoll::EpollEvent>(_loopThisHandlerLiveIn->getPoll(), weak_from_this(), m_socket->fd());
				connect();
			});
			m_retryTimer->setSingleShot(false);
			m_retryTimer->setInterval(1000);
			m_retryTimer->start();
		}
		reconnect();
	}
	else if (err == error::SocketError::E_INPROGRESS)
	{
		LOG_TRACE("connecting, wait for connect done");
	}
	else if (err == error::SocketError::E_NOERROR)
	{
		if (m_retryTimer)	// stop timer
		{
			m_retryTimer->stop();
			m_retryTimer = nullptr;
		}
		// clean up this first
		m_epollEvent->disable();

		auto connection = TcpConnection::makeTcpConnection(Application::loopManager()->dispatch(),
																   std::move(m_socket),
																   m_events, m_config);
		m_connectionEstablished.store(true, std::memory_order_relaxed);
		_connection = connection;
		LOG_TRACE("Connected to server");
		std::shared_ptr<Channel> channel = connection->getIOChannel();
		m_events.onConnected(channel);

		// extern life after remove
		volatile auto externLife = shared_from_this();
		// remove Connector from loop after connect success
		_loopThisHandlerLiveIn->removeEventHandlerFromLoop(shared_from_this());
	}
	else
	{
		LOG_WARN("other connect error", error::errorAsString(err));
		m_events.onError(err);
	}
}

std::shared_ptr<Connector> Connector::makeConnector(eventloop::EventLoop *loop, const Address &serverAddr,
													Events eventsPrototype, ConnectionConfig config)
{
	try
	{
		auto connector = make_shared<Connector>(loop, make_unique<socket::Socket>(serverAddr));
		connector->m_events = eventsPrototype;
		connector->m_epollEvent = make_unique<epoll::EpollEvent>(loop->getPoll(), connector, connector->m_socket->fd());
		connector->m_state = socket::TcpState::Closed;
		connector->m_config = config;

		return connector;
	}
	catch (error::SocketException &se)
	{
		eventsPrototype.onError(se.getErrorCode());
	}
	catch (error::ResourceLimitException &rle)
	{
		eventsPrototype.onError(rle.getSocketErrorCode());
	}
	return {};
}

void Connector::reconnect()
{
	m_epollEvent->disable();
	time::TimerInterval currentInterval = m_retryTimer->interval();
	if (currentInterval < 4000)
		m_retryTimer->setInterval(currentInterval * 2);
	LOG_INFO("Connector error on fd {}, retry in {} mseconds", m_socket->fd(), currentInterval);
}
}
