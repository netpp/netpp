#include "handlers/Acceptor.h"
#include "handlers/TcpConnection.h"
#include "Log.h"
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
		m_socket->listen();
	} catch (error::SocketException &se) {
		m_events->onError(se.getErrorCode());
	}
}

void Acceptor::handleRead()
{
	try {
		std::unique_ptr<socket::Socket> commingConnection = m_socket->accept();
		std::shared_ptr<Channel> channel = TcpConnection::makeTcpConnection(_dispatcher->dispatchEventLoop(),
																	std::move(commingConnection),
																	m_events->clone());
		SPDLOG_LOGGER_TRACE(logger, "New connection on Socket {}", m_socket->fd());
		m_events->onConnected(channel);
	} catch (error::SocketException &se) {
		m_events->onError(se.getErrorCode());
	} catch (error::ResourceLimitException &rle) {
		m_events->onError(rle.getSocketErrorCode());
	}
}

void Acceptor::handleWrite()
{}

void Acceptor::handleError()
{
	// TODO: will EPOLLERR happend in acceptor?
	m_events->onError(error::SocketError::E_EPOLLERR);
}

void Acceptor::handleClose()
{}

bool Acceptor::makeAcceptor(EventLoopDispatcher *dispatcher,
											Address listenAddr,
											std::unique_ptr<support::EventInterface> &&eventsPrototype)
{
	try {
		EventLoop *loop = dispatcher->dispatchEventLoop();
		auto acceptor = make_shared<Acceptor>(dispatcher, make_unique<socket::Socket>(listenAddr));
		auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), acceptor, acceptor->m_socket->fd());
		epoll::EpollEvent *eventPtr = event.get();
		acceptor->m_events = std::move(eventsPrototype);
		acceptor->m_epollEvent = std::move(event);

		eventPtr->setEnableRead(true);
		loop->addEventHandlerToLoop(acceptor);
		acceptor->listen();

		return true;
	} catch (error::SocketException &se) {
		eventsPrototype->onError(se.getErrorCode());
	} catch (error::ResourceLimitException &rle) {
		eventsPrototype->onError(rle.getSocketErrorCode());
	}
	return false;
}
}
