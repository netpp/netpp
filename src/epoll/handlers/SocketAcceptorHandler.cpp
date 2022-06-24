#include "epoll/handlers/SocketAcceptorHandler.h"
#include <utility>
#include <cassert>
#include "epoll/handlers/SocketConnectionHandler.h"
#include "support/Log.h"
#include "eventloop/EventLoopManager.h"
#include "error/Exception.h"
#include "location/Address.h"
#include "eventloop/EventLoop.h"
#include "Application.h"
#include "iodevice/TcpSocket.h"

using std::make_unique;
using std::make_shared;

namespace netpp {
SocketAcceptorHandler::SocketAcceptorHandler(EventLoop *loop, Address address)
		: EpollEventHandler(loop), m_address{std::move(address)}, m_state{TcpState::Closed}
{
	m_socket = std::make_unique<TcpSocket>();
	m_socket->open();
}

SocketAcceptorHandler::~SocketAcceptorHandler() = default;

int SocketAcceptorHandler::fileDescriptor() const
{
	return m_socket->fileDescriptor();
}

void SocketAcceptorHandler::listen()
{
	_loopThisHandlerLiveIn->runInLoop([this, weakAcceptor{weak_from_this()}]{
		auto acceptor = weakAcceptor.lock();
		if (acceptor)
		{
			try
			{
				// only listen at initial state
				if (m_state == TcpState::Closed)
				{
					activeEvents(EpollEv::IN);
					m_socket->bind(m_address);
					m_socket->listen();
					m_state = TcpState::Listen;
				}
			}
			catch (InternalException &e)
			{
				if (m_errorCallback)
					m_errorCallback(e.getErrorCode());
			}
		}
	});
}

void SocketAcceptorHandler::handleIn()
{
	try
	{
		assert(Application::instance());
		std::unique_ptr<SocketDevice> comingConnection(m_socket->accept());
		if (comingConnection)
		{
			auto connection = std::make_shared<SocketConnectionHandler>(Application::loopManager()->dispatch(),
																		comingConnection);
			connection->init();
			std::shared_ptr<Channel> channel = connection->getIOChannel();
			LOG_TRACE("New connection on Socket {}", m_socket->fd());
			if (m_connectedCallback)
				m_connectedCallback(channel);
		}
		else
		{
			m_errorCallback(Error::ConnectRefused);
		}
	}
	catch (InternalException &e)
	{
		if (m_errorCallback)
			m_errorCallback(e.getErrorCode());
	}
}
}
