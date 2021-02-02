//
// Created by gaojian on 2020/7/20.
//

#include "TcpClient.h"
#include "handlers/Connector.h"
#include "EventLoopDispatcher.h"

using std::make_unique;

namespace netpp {
TcpClient::TcpClient(EventLoopDispatcher *dispatcher, Address addr, std::unique_ptr<support::EventInterface> &&eventsPrototype)
	: _dispatcher{dispatcher}, m_addr{addr}, m_eventsPrototype{std::move(eventsPrototype)}
{}

void TcpClient::connect()
{
	handlers::Connector::makeConnector(_dispatcher, m_addr, m_eventsPrototype->clone());
}

void TcpClient::disconnect()
{
	// TODO: impl disconnect for client
}
}
