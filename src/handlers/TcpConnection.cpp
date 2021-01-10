#include "handlers/TcpConnection.h"
#include "Log.h"
#include "EventLoop.h"
#include "Channel.h"

using std::make_unique;
using std::make_shared;

namespace netpp::handlers {
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

void TcpConnection::handleClose()
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
