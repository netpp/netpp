#include "handlers/Connector.h"
#include "Log.h"
#include "handlers/TcpConnection.h"
#include "EventLoop.h"
#include "EventLoopDispatcher.h"
#include "error/Exception.h"
#include "error/SocketError.h"

using std::make_unique;
using std::make_shared;

namespace netpp::handlers {
Connector::Connector(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket) noexcept
		: EventHandler(socket->fd()), _dispatcher(dispatcher), m_socket{std::move(socket)}
{}

void Connector::connect() noexcept
{
	try {
		m_socket->connect();
		// connect may success immediately, the writeable event may not triggered
		m_epollEvent->setEnableWrite(true);
	} catch (error::SocketException &se) {
		error::SocketError code = se.getErrorCode();
		if (code != error::SocketError::E_INPROGRESS)
			m_events->onError(code);
	}
}

void Connector::handleRead() noexcept
{}

void Connector::handleWrite() noexcept
{
	SPDLOG_LOGGER_TRACE(logger, "Connector write available");
	error::SocketError err = m_socket->getError();
	// TODO: maybe can retry in other situations
	if (err == error::SocketError::E_TIMEDOUT)
	{
		SPDLOG_LOGGER_ERROR(logger, "Connector error {}", error::errorAsString(err));
		if (!m_retryTimer)
			m_retryTimer = make_unique<Timer>(EventLoop::thisLoop());
		reconnect();
	}
	else if (err == error::SocketError::E_INPROGRESS)
	{
		SPDLOG_LOGGER_TRACE(logger, "connecting, wait for connect done");
	}
	else	// connect success or other error
	{
		if (m_retryTimer)
		{
			m_retryTimer->stop();
			m_retryTimer = nullptr;
		}
		std::shared_ptr<Channel> channel = TcpConnection::makeTcpConnection(_dispatcher->dispatchEventLoop(),
																   std::move(m_socket),
																   m_events->clone()
		);
		SPDLOG_LOGGER_TRACE(logger, "Connected to server");
		m_events->onConnected(channel);
	}
}

void Connector::handleError() noexcept
{
	// TODO: will EPOLLERR happend in acceptor?
	m_events->onError(error::SocketError::E_EPOLLERR);
}

void Connector::handleClose() noexcept
{}

bool Connector::makeConnector(EventLoopDispatcher *dispatcher,
											 Address serverAddr,
											 std::unique_ptr<support::EventInterface> &&eventsPrototype) noexcept
{
	try {
		EventLoop *loop = dispatcher->dispatchEventLoop();
		auto connector = make_shared<Connector>(dispatcher, make_unique<socket::Socket>(serverAddr));
		auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connector);
		auto eventPtr = event.get();
		connector->m_events = std::move(eventsPrototype);
		connector->m_epollEvent = std::move(event);

		eventPtr->setEnableWrite(true);
		loop->addEventHandlerToLoop(connector);
		connector->connect();

		return true;
	} catch (error::SocketException &se) {
		eventsPrototype->onError(se.getErrorCode());
	} catch (error::ResourceLimitException &rle) {
		eventsPrototype->onError(rle.getErrorCode());
	}
	return false;
}

void Connector::reconnect() noexcept
{
	try {
		m_epollEvent->disableEvents();
		m_retryTimer->setOnTimeout([=]{
			auto oldSocket = std::move(m_socket);
			m_socket = make_unique<socket::Socket>(oldSocket->getAddr());
			_fd = m_socket->fd();
			m_epollEvent->setEnableWrite(true);
			connect();
		});
		unsigned currentInterval = m_retryTimer->interval();
		if (currentInterval == 0)
			m_retryTimer->setInterval(100);
		else if (currentInterval < 4000)
			m_retryTimer->setInterval(currentInterval * 2);
		m_retryTimer->start();
		SPDLOG_LOGGER_INFO(logger, "Connector error on fd {}, retry in {} mseconds", m_socket->fd(), currentInterval);
	} catch (error::SocketException &se) {
		m_events->onError(se.getErrorCode());
	} catch (error::ResourceLimitException &rle) {
		m_events->onError(rle.getErrorCode());
	}
}
}
