//
// Created by gaojian on 2020/7/24.
//

#include "TcpClient.h"
#include "Events.h"
#include "EventLoopDispatcher.h"
#include <iostream>
#include "channel/Channel.h"

class Echo {
public:
	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		auto reader = channel->reader();
		std::size_t size = reader.readableBytes();
		std::string data = reader.retrieveString(size).value();
		std::cout << "Received size "<< size << " data " << data;
		std::string str;
		std::cin >> str;
		auto writer = channel->writer();
		writer.writeString(str);
		channel->send();
	}

	void onConnected(std::shared_ptr<netpp::Channel> channel)
	{
		auto writer = channel->writer();
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
	netpp::EventLoopDispatcher dispatcher;
	netpp::Events events(std::make_shared<Echo>());
	netpp::TcpClient client(&dispatcher, netpp::Address("127.0.0.1", 12345), std::move(events));
	client.connect();
	dispatcher.startLoop();
	return 0;
}
