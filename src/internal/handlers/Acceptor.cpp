#include "internal/handlers/Acceptor.h"
#include "internal/handlers/TcpConnection.h"
#include "internal/support/Log.h"
#include "eventloop/EventLoopManager.h"
#include "error/Exception.h"
#include "Address.h"
#include "internal/socket/Socket.h"
#include "internal/socket/SocketEnums.h"
#include "eventloop/EventLoop.h"
#include "Application.h"

using std::make_unique;
using std::make_shared;

namespace netpp::internal::handlers {
Acceptor::Acceptor(eventloop::EventLoop *loop, std::unique_ptr<socket::Socket> &&socket)
		: epoll::EventHandler(loop), m_socket{std::move(socket)}, m_state{socket::TcpState::Closed}
{}

Acceptor::~Acceptor() = default;

void Acceptor::listen()
{
	auto acceptor = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([acceptor]{
		try
		{
			// only listen at initial state
			if (acceptor->m_state == socket::TcpState::Closed)
			{
				acceptor->_loopThisHandlerLiveIn->addEventHandlerToLoop(acceptor);
				acceptor->m_epollEvent->active(epoll::EpollEv::IN);
				acceptor->m_socket->listen();
				acceptor->m_state = socket::TcpState::Listen;
			}
		}
		catch (error::SocketException &se)
		{
			acceptor->m_events.onError(se.getErrorCode());
		}
	});
}

void Acceptor::stop()
{
	// extern life after remove
	auto externLife = shared_from_this();
	_loopThisHandlerLiveIn->runInLoop([externLife]{
		externLife->m_epollEvent->disable();
		externLife->_loopThisHandlerLiveIn->removeEventHandlerFromLoop(externLife);
		externLife->m_state = socket::TcpState::Closed;
	});
}

void Acceptor::handleIn()
{
	try
	{
		std::unique_ptr<socket::Socket> comingConnection = m_socket->accept();
		auto connection = TcpConnection::makeTcpConnection(Application::loopManager()->dispatch(),
																	std::move(comingConnection),
																	m_events, m_config);
		std::shared_ptr<Channel> channel = connection->getIOChannel();	// connection ptr will not expire here
		LOG_TRACE("New connection on Socket {}", m_socket->fd());
		m_events.onConnected(channel);
	}
	catch (error::SocketException &se)
	{
		m_events.onError(se.getErrorCode());
	}
	catch (error::ResourceLimitException &rle)
	{
		m_events.onError(rle.getSocketErrorCode());
	}
}

std::shared_ptr<Acceptor> Acceptor::makeAcceptor(eventloop::EventLoop *loop, const Address &listenAddr,
											Events eventsPrototype, ConnectionConfig config)
{
	try
	{
		auto acceptor = make_shared<Acceptor>(loop, make_unique<socket::Socket>(listenAddr));
		auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), acceptor, acceptor->m_socket->fd());
		acceptor->m_events = eventsPrototype;
		acceptor->m_epollEvent = std::move(event);
		acceptor->m_state = socket::TcpState::Closed;
		acceptor->m_config = config;

		return acceptor;
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
}
