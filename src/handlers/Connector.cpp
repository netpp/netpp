#include "handlers/Connector.h"
#include "Log.h"
#include "handlers/TcpConnection.h"
#include "EventLoop.h"
#include "EventLoopDispatcher.h"

using std::make_unique;
using std::make_shared;

namespace netpp::handlers {
Connector::Connector(EventLoopDispatcher *dispatcher, std::unique_ptr<support::Socket> &&socket)
		: EventHandler(socket->fd()), _dispatcher(dispatcher), m_socket{std::move(socket)}
{}

void Connector::connect()
{
	m_socket->connect();
}

void Connector::handleRead()
{}

void Connector::handleWrite()
{
	SPDLOG_LOGGER_TRACE(logger, "Connector write available");
	int err = m_socket->getError();
	if (err)
	{
		SPDLOG_LOGGER_ERROR(logger, "Connector error {}", std::string(strerror(errno)));
		if (!m_retryTimer)
			m_retryTimer = make_unique<Timer>(EventLoop::thisLoop());
		reconnect();
	}
	else	// connect success
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

void Connector::handleError()
{
	SPDLOG_LOGGER_ERROR(logger, "Connector error {}", std::string(strerror(errno)));
	m_events->onError();
}

void Connector::handleClose()
{}

bool Connector::makeConnector(EventLoopDispatcher *dispatcher,
											 Address serverAddr,
											 std::unique_ptr<support::EventInterface> &&eventsPrototype)
{
	EventLoop *loop = dispatcher->dispatchEventLoop();
	auto connector = make_shared<Connector>(dispatcher, make_unique<support::Socket>(serverAddr));
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connector);
	auto eventPtr = event.get();
	connector->m_events = std::move(eventsPrototype);
	connector->m_epollEvent = std::move(event);

	eventPtr->setEnableWrite(true);
	loop->addEventHandlerToLoop(connector);
	connector->connect();

	return true;
}

void Connector::reconnect()
{
	m_epollEvent->disableEvents();
	m_retryTimer->setOnTimeout([=]{
		auto oldSocket = std::move(m_socket);
		// FIXME: crate new socket may failed
		m_socket = make_unique<support::Socket>(oldSocket->getAddr());
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
}
}
