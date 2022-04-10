//
// Created by gaojian on 2020/7/24.
//

#include "TcpClient.h"
#include "Events.h"
#include <iostream>
#include "channel/TcpChannel.h"
#include "Application.h"

class Echo {
public:
	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		auto tcpChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(channel);
		auto reader = tcpChannel->reader();
		std::size_t size = reader.readableBytes();
		std::string data = reader.retrieveString(size).value();
		std::cout << "Received size "<< size << " data " << data;
		std::string str;
		std::cin >> str;
		auto writer = tcpChannel->writer();
		writer.writeString(str);
		channel->send();
	}

	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{
		auto tcpChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(channel);
		auto writer = tcpChannel->writer();
		writer.writeString("hello netpp");
		channel->send();
	}

	void onWriteCompleted([[maybe_unused]] std::shared_ptr<netpp::Channel> channel)
	{
		std::cout << "Write completed";
	}

	void onDisconnect([[maybe_unused]] std::shared_ptr<netpp::Channel> channel)
	{
		std::cout << "Disconnected from server, exit";
		exit(0);
	}
};

int main()
{
	netpp::Config config;
	config.tickTimer.enable = false;
	config.eventHandler = netpp::Events(std::make_shared<Echo>());
	netpp::Application app(config);
	netpp::TcpClient client(netpp::Address("127.0.0.1", 12347));
	client.connect();
	app.exec();
}
