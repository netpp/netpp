#include "handlers/Acceptor.h"
#include "handlers/TcpConnection.h"
#include "Log.h"
#include "EventLoopDispatcher.h"
#include "error/Exception.h"
#include "error/SocketError.h"

using std::make_unique;
using std::make_shared;

namespace netpp::handlers {
Acceptor::Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket) noexcept
		: EventHandler(socket->fd()), _dispatcher{dispatcher}, m_socket{std::move(socket)}
{}

void Acceptor::listen() noexcept
{
	try {
		m_socket->listen();
	} catch (error::SocketException &se) {
		m_events->onError(se.getErrorCode());
	}
}

void Acceptor::handleRead() noexcept
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
		m_events->onError(rle.getErrorCode());
	}
}

void Acceptor::handleWrite() noexcept
{}

void Acceptor::handleError() noexcept
{
	// TODO: will EPOLLERR happend in acceptor?
	m_events->onError(error::SocketError::E_EPOLLERR);
}

void Acceptor::handleClose() noexcept
{}

bool Acceptor::makeAcceptor(EventLoopDispatcher *dispatcher,
											Address listenAddr,
											std::unique_ptr<support::EventInterface> &&eventsPrototype) noexcept
{
	try {
		EventLoop *loop = dispatcher->dispatchEventLoop();
		auto acceptor = make_shared<Acceptor>(dispatcher, make_unique<socket::Socket>(listenAddr));
		auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), acceptor);
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
		eventsPrototype->onError(rle.getErrorCode());
	}
	return false;
}
}
