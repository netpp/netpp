//
// Created by gaojian on 2020/7/20.
//

#include "TcpClient.h"
#include "internal/handlers/Connector.h"
#include "internal/handlers/TcpConnection.h"
#include "Application.h"

using std::make_unique;

namespace netpp {
TcpClient::TcpClient(Address address)
	: m_address{std::move(address)}
{
	Config config = Application::appConfig();
	m_config = config.connection;
	m_eventsPrototype = config.eventHandler;
}

TcpClient::~TcpClient()
{
	disconnect();
}

void TcpClient::connect()
{
	auto connector = internal::handlers::Connector::makeConnector(Application::loopManager()->dispatch(), m_address, m_eventsPrototype, m_config);
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
