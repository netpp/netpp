#include "handlers/Acceptor.h"
#include "handlers/TcpConnection.h"
#include "Log.h"
#include "EventLoopDispatcher.h"

using std::make_unique;
using std::make_shared;

namespace netpp::handlers {
Acceptor::Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket)
		: EventHandler(socket->fd()), _dispatcher{dispatcher}, m_socket{std::move(socket)}
{}

void Acceptor::listen()
{
	m_socket->listen();
}

void Acceptor::handleRead()
{
	// FIXME: accept may failed due to fd limit
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

void Acceptor::handleClose()
{}

bool Acceptor::makeAcceptor(EventLoopDispatcher *dispatcher,
											Address listenAddr,
											std::unique_ptr<support::EventInterface> &&eventsPrototype)
{
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
}
}
