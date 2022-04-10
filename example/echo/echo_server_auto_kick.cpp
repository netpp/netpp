#include "TcpServer.h"
#include "Events.h"
#include "channel/TcpChannel.h"
#include "Application.h"
#include <iostream>

class Echo {
public:
	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		auto tcpChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(channel);
		auto reader = tcpChannel->reader();
		std::size_t size = reader.readableBytes();
		std::string data = reader.retrieveString(size).value();
		std::cout << "received " << data << " on channel" << std::endl;
		auto writer = tcpChannel->writer();
		writer.writeString(data);
		channel->send();
	}
};

int main()
{
	netpp::Config config;
	config.connection.enableAutoClose = true;
	config.connection.idleConnectionTimeout = 8;
	config.connection.halfCloseConnectionTimeout = 8;
	config.eventHandler = netpp::Events(std::make_shared<Echo>());
	netpp::Application app(config);
	netpp::TcpServer server(netpp::Address("0.0.0.0", 12347));
	server.listen();
	app.exec();
}
