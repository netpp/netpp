#include "handlers/Acceptor.h"
#include "handlers/TcpConnection.h"
#include "support/Log.h"
#include "EventLoopDispatcher.h"
#include "error/Exception.h"
#include "error/SocketError.h"

using std::make_unique;
using std::make_shared;

namespace netpp::handlers {
Acceptor::Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket)
		: _dispatcher{dispatcher}, m_socket{std::move(socket)}
{}

void Acceptor::listen()
{
	try {
		m_epollEvent->setEnableRead(true);
		m_socket->listen();
	} catch (error::SocketException &se) {
		m_events.onError(se.getErrorCode());
	}
}

void Acceptor::stop()
{
	m_epollEvent->disableEvents();
	// extern life after remove
	volatile auto externLife = shared_from_this();
	EventLoop::thisLoop()->removeEventHandlerFromLoop(shared_from_this());
}

void Acceptor::handleRead()
{
	try {
		std::unique_ptr<socket::Socket> commingConnection = m_socket->accept();
		auto connection = TcpConnection::makeTcpConnection(_dispatcher->dispatchEventLoop(),
																	std::move(commingConnection),
																	m_events).lock();
		std::shared_ptr<Channel> channel = connection->getIOChannel();	// connection ptr will not expire here
		LOG_TRACE("New connection on Socket {}", m_socket->fd());
		m_events.onConnected(channel);
	} catch (error::SocketException &se) {
		m_events.onError(se.getErrorCode());
	} catch (error::ResourceLimitException &rle) {
		m_events.onError(rle.getSocketErrorCode());
	}
}

void Acceptor::handleWrite()
{}

void Acceptor::handleError()
{
	// TODO: will EPOLLERR happend in acceptor?
	m_events.onError(error::SocketError::E_EPOLLERR);
}

void Acceptor::handleClose()
{}

std::weak_ptr<Acceptor> Acceptor::makeAcceptor(EventLoopDispatcher *dispatcher,
											Address listenAddr,
											Events eventsPrototype)
{
	try {
		EventLoop *loop = dispatcher->dispatchEventLoop();
		auto acceptor = make_shared<Acceptor>(dispatcher, make_unique<socket::Socket>(listenAddr));
		auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), acceptor, acceptor->m_socket->fd());
		// epoll::EpollEvent *eventPtr = event.get();
		acceptor->m_events = eventsPrototype;
		acceptor->m_epollEvent = std::move(event);
		acceptor->m_addr = listenAddr;

		loop->addEventHandlerToLoop(acceptor);

		return acceptor;
	} catch (error::SocketException &se) {
		eventsPrototype.onError(se.getErrorCode());
	} catch (error::ResourceLimitException &rle) {
		eventsPrototype.onError(rle.getSocketErrorCode());
	}
	return std::weak_ptr<Acceptor>();
}
}
