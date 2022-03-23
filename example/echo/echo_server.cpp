//
// Created by gaojian on 2020/7/5.
//

#include "TcpServer.h"
#include "Events.h"
#include "channel/TcpChannel.h"
#include "EventLoopDispatcher.h"
#include <iostream>

class Echo {
public:
	void onMessageReceived(std::shared_ptr<netpp::Channel> channel)
	{
		auto tcpChannel = std::dynamic_pointer_cast<netpp::TcpChannel>(channel);
		auto reader = tcpChannel->reader();
		std::size_t size = reader.readableBytes();
		std::string data = reader.retrieveString(size).value();
		std::cout << "received " << data << " on channel";
		auto writer = tcpChannel->writer();
		writer.writeString(data);
		channel->send();
	}
};

int main()
{
	netpp::EventLoopDispatcher dispatcher;
	netpp::Events events(std::make_shared<Echo>());
	netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(events));
	server.listen();
	dispatcher.startLoop();
}
