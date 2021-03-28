#include "internal/handlers/Acceptor.h"
#include "internal/handlers/TcpConnection.h"
#include "internal/support/Log.h"
#include "EventLoopDispatcher.h"
#include "error/Exception.h"
#include "Address.h"
#include "internal/socket/Socket.h"
#include "internal/socket/SocketEnums.h"
#include "EventLoop.h"

using std::make_unique;
using std::make_shared;

namespace netpp::internal::handlers {
Acceptor::Acceptor(EventLoopDispatcher *dispatcher, std::unique_ptr<socket::Socket> &&socket)
		: _dispatcher{dispatcher}, m_socket{std::move(socket)}, m_state{socket::TcpState::Closed}
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
				acceptor->m_epollEvent->active(epoll::Event::IN);
				acceptor->m_socket->listen();
				acceptor->m_state = socket::TcpState::Established;
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
		auto connection = TcpConnection::makeTcpConnection(_dispatcher->dispatchEventLoop(),
																	std::move(comingConnection),
																	m_events).lock();
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

std::shared_ptr<Acceptor> Acceptor::makeAcceptor(EventLoopDispatcher *dispatcher,
											const Address &listenAddr,
											Events eventsPrototype)
{
	try
	{
		EventLoop *loop = dispatcher->dispatchEventLoop();
		auto acceptor = make_shared<Acceptor>(dispatcher, make_unique<socket::Socket>(listenAddr));
		auto event = make_unique<epoll::EpollEvent>(loop->getPoll(), acceptor, acceptor->m_socket->fd());
		acceptor->m_events = eventsPrototype;
		acceptor->m_epollEvent = std::move(event);
		acceptor->_loopThisHandlerLiveIn = loop;
		acceptor->m_state = socket::TcpState::Closed;

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
	return std::shared_ptr<Acceptor>();
}
}
