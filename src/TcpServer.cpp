//
// Created by gaojian on 2020/7/6.
//

#include "TcpServer.h"
#include "EventLoop.h"
#include "handlers/Acceptor.h"
#include "EventLoopDispatcher.h"
#include "handlers/SignalHandler.h"

namespace netpp {
TcpServer::TcpServer(EventLoopDispatcher *dispatcher, std::unique_ptr<support::EventInterface> &&eventsPrototype)
	: _acceptorChannel{nullptr}, _dispatcher(dispatcher),
	  _eventPrototype{std::move(eventsPrototype)}
{}

void TcpServer::listen(Address listenAddr)
{
	handlers::Acceptor::makeAcceptor(_dispatcher, listenAddr, _eventPrototype->clone());
	handlers::SignalHandler::makeSignalHandler(_dispatcher->dispatchEventLoop(), std::move(_eventPrototype));
	_eventPrototype = nullptr;
}
}
