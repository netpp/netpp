#include "epoll/handlers/SocketAcceptorHandler.h"
#include <utility>
#include <cassert>
#include "support/Log.h"
#include "eventloop/EventLoopManager.h"
#include "error/Exception.h"
#include "location/Address.h"
#include "eventloop/EventLoop.h"
#include "Application.h"
#include "iodevice/TcpSocket.h"
#include "support/Util.h"
#include "channel/TcpChannel.h"

using std::make_unique;
using std::make_shared;

namespace netpp {
SocketAcceptorHandler::SocketAcceptorHandler(EventLoop *loop)
		: EpollEventHandler(loop), m_state{TcpState::Closed}
{
	m_socket = std::make_unique<TcpSocket>();
	m_socket->open();
}

SocketAcceptorHandler::~SocketAcceptorHandler() = default;

int SocketAcceptorHandler::fileDescriptor() const
{
	return m_socket->fileDescriptor();
}

void SocketAcceptorHandler::listen(const Address &address)
{
	_loopThisHandlerLiveIn->runInLoop([this, weakAcceptor{weak_from_this()}, address]{
		auto acceptor = weakAcceptor.lock();
		if (acceptor)
		{
			try
			{
				// only listen at initial state
				if (m_state == TcpState::Closed)
				{
					activeEvents(EpollEv::IN);
					m_socket->bind(address);
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

void SocketAcceptorHandler::setConnectedCallback(const ConnectedCallBack &cb)
{
	_loopThisHandlerLiveIn->runInLoop([this, connection{shared_from_this()}, cb] {
		m_connectedCallback = cb;
	});
}

void SocketAcceptorHandler::setErrorCallback(const ErrorCallBack &cb)
{
	_loopThisHandlerLiveIn->runInLoop([this, connection{shared_from_this()}, cb] {
		m_errorCallback = cb;
	});
}

void SocketAcceptorHandler::handleIn()
{
	try
	{
		APPLICATION_INSTANCE_REQUIRED();
		SocketDevice *comingConnection = m_socket->accept();
		if (comingConnection)
		{
			auto targetLoop = Application::loopManager()->dispatch();
			// create connection in target event loop
			targetLoop->runInLoop([targetLoop, comingConnection, this, weakAcceptor{weak_from_this()}]{
				std::unique_ptr<TcpSocket> socket;
				socket.reset(dynamic_cast<TcpSocket *>(comingConnection));
				std::shared_ptr<Channel> channel = std::make_shared<TcpChannel>(targetLoop, std::move(socket));
				LOG_TRACE("New connection on Socket {}", m_socket->fd());
				auto acceptor = weakAcceptor.lock();
				if (acceptor)
				{
					// run callback in acceptor loop
					_loopThisHandlerLiveIn->runInLoop([this, weakAcceptor{weak_from_this()}, channel]{
						auto acceptor = weakAcceptor.lock();
						if (acceptor && m_connectedCallback)
							m_connectedCallback(channel);
					});
				}
			});
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
