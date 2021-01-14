//
// Created by gaojian on 2020/7/20.
//

#include "TcpClient.h"
#include "handlers/Connector.h"
#include "EventLoopDispatcher.h"
#include "handlers/SignalHandler.h"

using std::make_unique;

namespace netpp {
TcpClient::TcpClient(EventLoopDispatcher *dispatcher, std::unique_ptr<support::EventInterface> &&eventsPrototype)
	: _dispatcher{dispatcher}, m_eventsPrototype{std::move(eventsPrototype)}
{}

void TcpClient::connect(Address serverAddr)
{
	handlers::Connector::makeConnector(_dispatcher, serverAddr, m_eventsPrototype->clone());
	handlers::SignalHandler::makeSignalHandler(_dispatcher->dispatchEventLoop(), m_eventsPrototype->clone());
}

void TcpClient::disconnect(Address serverAddr)
{
	// TODO: impl disconnect for client
}
}
