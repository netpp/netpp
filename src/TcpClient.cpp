//
// Created by gaojian on 2020/7/20.
//

#include "TcpClient.h"
#include "handlers/Connector.h"
#include "handlers/TcpConnection.h"
#include "EventLoopDispatcher.h"

using std::make_unique;

namespace netpp {
TcpClient::TcpClient(EventLoopDispatcher *dispatcher, Address addr, Events eventsPrototype)
	: _dispatcher{dispatcher}, m_addr{addr}, m_eventsPrototype{eventsPrototype}
{}

TcpClient::~TcpClient()
{
	disconnect();
}

void TcpClient::connect()
{
	auto m_connector = internal::handlers::Connector::makeConnector(_dispatcher, m_addr, m_eventsPrototype).lock();
	m_connector->connect();
}

void TcpClient::disconnect()
{
	if (m_connector)
	{
		if (m_connector->connected())
		{
			auto connection = m_connector->getConnection().lock();
			if (connection)
				connection->closeAfterWriteCompleted();
		}
		else
			m_connector->stopConnect();
		m_connector = nullptr;
	}
}
}
