//
// Created by gaojian on 2020/8/9.
//

#include "handlers/SocketEventHandler.h"
#include "EventLoopDispatcher.h"
#include <memory>
#include "Log.h"
#include "Channel.h"

using std::make_unique;
using std::make_shared;

namespace netpp::handlers {
Acceptor::Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<Socket> &&socket)
		: EventHandler(socket->fd()), _dispatcher{dispatcher}, m_socket{std::move(socket)}
{}

void Acceptor::listen()
{
	m_socket->listen();
}

void Acceptor::handleRead()
{
	std::shared_ptr<Channel> channel = TcpConnection::makeTcpConnection(_dispatcher->dispatchEventLoop(),
															   std::move(m_socket->accept()),
															   m_events->clone());
	SPDLOG_LOGGER_TRACE(logger, "New connection on Socket {}", m_socket->fd());
	m_events->onConnected(channel);
}

void Acceptor::handleWrite()
{}

void Acceptor::handleError()
{
	m_events->onError();
}

void Acceptor::handleDisconnect()
{}

bool Acceptor::makeAcceptor(EventLoopDispatcher *dispatcher,
											Address listenAddr,
											std::unique_ptr<Events> &&eventsPrototype)
{
	EventLoop *loop = dispatcher->dispatchEventLoop();
	auto acceptor = make_shared<Acceptor>(dispatcher, make_unique<Socket>(listenAddr));
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), acceptor);
	epoll::EpollEvent *eventPtr = event.get();
	acceptor->m_events = std::move(eventsPrototype);
	acceptor->m_epollEvent = std::move(event);

	loop->addEventHandlerToLoop(acceptor);
	eventPtr->setEnableRead(true);
	acceptor->listen();

	return true;
}

// Connector
Connector::Connector(EventLoopDispatcher *dispatcher, std::unique_ptr<Socket> &&socket)
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

void Connector::handleDisconnect()
{}

bool Connector::makeConnector(EventLoopDispatcher *dispatcher,
											 Address serverAddr,
											 std::unique_ptr<Events> &&eventsPrototype)
{
	EventLoop *loop = dispatcher->dispatchEventLoop();
	auto connector = make_shared<Connector>(dispatcher, make_unique<Socket>(serverAddr));
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connector);
	auto eventPtr = event.get();
	connector->m_events = std::move(eventsPrototype);
	connector->m_epollEvent = std::move(event);

	loop->addEventHandlerToLoop(connector);
	eventPtr->setEnableWrite(true);
	connector->connect();

	return true;
}

void Connector::reconnect()
{
	m_epollEvent->disableEvents();
	m_retryTimer->setOnTimeout([=]{
		auto oldSocket = std::move(m_socket);
		m_socket = make_unique<Socket>(oldSocket->getAddr());
		m_fd = m_socket->fd();
		m_epollEvent->setEnableWrite(true);
		connect();
	});
	unsigned currentInterval = m_retryTimer->interval();
	if (currentInterval == 0)
		m_retryTimer->setInterval(500);
	else if (currentInterval < 4000)
		m_retryTimer->setInterval(currentInterval * 2);
	m_retryTimer->start();
	SPDLOG_LOGGER_INFO(logger, "Connector error on fd {}, retry in {} mseconds", m_socket->fd(), currentInterval);
}

// TcpConnection
TcpConnection::TcpConnection(std::unique_ptr<Socket> &&socket, EventLoop *loop)
		: EventHandler(socket->fd()), _loop{loop}, m_state{TcpState::Connected}, 
		m_isWaitWriting{false}, m_socket{std::move(socket)}, 
		m_writeBuffer{make_shared<ByteArray>()}, m_receiveBuffer{make_shared<ByteArray>()}
{}

void TcpConnection::handleRead()
{
	SocketIO::read(m_socket.get(), m_receiveBuffer);
	SPDLOG_LOGGER_TRACE(logger, "Available size", m_receiveBuffer->readableBytes());
	auto channel = make_shared<Channel>(shared_from_this(), m_writeBuffer, m_receiveBuffer);
	m_events->onMessageReceived(channel);
}

void TcpConnection::handleWrite()
{
	SocketIO::write(m_socket.get(), m_writeBuffer);
	m_epollEvent->setEnableWrite(false);
	m_events->onWriteCompleted();
	m_isWaitWriting = false;
	if (m_state == TcpState::Disconnecting)
		m_socket->shutdownWrite();
}

void TcpConnection::handleError()
{
	SPDLOG_LOGGER_ERROR(logger, "Socket {} error", m_socket->fd());
	m_events->onError();
}

void TcpConnection::handleDisconnect()
{
	SPDLOG_LOGGER_TRACE(logger, "Socket {} disconnected", m_socket->fd());
	m_events->onDisconnect();
	m_epollEvent->disableEvents();
	// extern TcpConnection life after remove
	auto externLife = shared_from_this();
	EventLoop::thisLoop()->removeEventHandlerFromLoop(shared_from_this());
	m_state = TcpState::Disconnected;
}

void TcpConnection::sendInLoop()
{
	m_isWaitWriting = true;
	m_epollEvent->setEnableWrite(true);
}

void TcpConnection::closeAfterWriteCompleted()
{
	// nothing to write, close direcly
	if (!m_isWaitWriting)
		m_socket->shutdownWrite();
	m_state = TcpState::Disconnecting;
}

std::shared_ptr<Channel> TcpConnection::makeTcpConnection(EventLoop *loop, std::unique_ptr<Socket> &&socket,
												 std::unique_ptr<Events> &&eventsPrototype)
{
	auto connection = std::make_shared<TcpConnection>(std::move(socket), loop);
	auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), connection);
	epoll::EpollEvent *eventPtr = event.get();
	connection->m_epollEvent = std::move(event);
	connection->m_events = std::move(eventsPrototype);
	eventPtr->setEnableWrite(false);
	eventPtr->setEnableRead(true);
	loop->addEventHandlerToLoop(connection);
	return make_shared<Channel>(connection, connection->m_writeBuffer, connection->m_receiveBuffer);
}
}
