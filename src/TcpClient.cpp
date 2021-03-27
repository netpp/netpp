//
// Created by gaojian on 2020/7/20.
//

#include "TcpClient.h"
#include "internal/handlers/Connector.h"
#include "internal/handlers/TcpConnection.h"
#include "EventLoopDispatcher.h"

using std::make_unique;

namespace netpp {
TcpClient::TcpClient(EventLoopDispatcher *dispatcher, Address addr, Events eventsPrototype)
	: m_addr{std::move(addr)}, _dispatcher{dispatcher}, m_eventsPrototype{std::move(eventsPrototype)}
{}

TcpClient::~TcpClient()
{
	disconnect();
}

void TcpClient::connect()
{
	auto connector = internal::handlers::Connector::makeConnector(_dispatcher, m_addr, m_eventsPrototype);
	connector->connect();
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
